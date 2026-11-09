import hashlib
import json
import os
import queue
import threading
import time
from datetime import datetime, timezone
from pathlib import Path

APP_SUPPORT = Path.home() / "Library" / "Application Support" / "T425-SIM Control Center"
DIAGNOSTIC_EXECUTABLE_SHA256 = "8378123e6197952e9e174fd8105ed8e5b36eb3aae12767495b6319c48134d02e"


def select_iserver_executable(app_root=None, environ=None):
    """Select the normal runner by default, or only the pinned diagnostic binary."""
    env = os.environ if environ is None else environ
    root = Path(app_root or Path(__file__).resolve().parents[1]).resolve()
    default_path = root / "build" / "bin" / "iserver-linkusb"
    expected_diag = root / "build" / "bin" / "iserver-linkusb-sp-reply-diag"
    requested = env.get("T425_ISERVER_EXECUTABLE")
    if requested:
        selected = Path(requested).expanduser().resolve(strict=True)
        expected = expected_diag.resolve(strict=True)
        if selected != expected:
            raise RuntimeError("T425_ISERVER_EXECUTABLE deve apontar para o executável diagnóstico desta variante: " + str(expected))
        if not selected.is_file() or not os.access(str(selected), os.X_OK):
            raise RuntimeError("Executável diagnóstico ausente ou sem permissão de execução: " + str(selected))
        digest = hashlib.sha256(selected.read_bytes()).hexdigest()
        if digest != DIAGNOSTIC_EXECUTABLE_SHA256:
            raise RuntimeError("SHA-256 do IServer diagnóstico diverge do binário aprovado para esta variante: " + digest)
        return selected, digest, "ENV_DIAGNOSTIC"

    selected = default_path.resolve(strict=True)
    if not selected.is_file() or not os.access(str(selected), os.X_OK):
        raise RuntimeError("Executável IServer padrão ausente ou sem permissão de execução: " + str(selected))
    digest = hashlib.sha256(selected.read_bytes()).hexdigest()
    return selected, digest, "DEFAULT"

def utc_now():
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds").replace("+00:00", "Z")

class MonitorController:
    def __init__(self, event_path=None):
        self.commands = queue.Queue()
        self.events = queue.Queue()
        self.state_lock = threading.RLock()
        self.state = {
            "linkusb": "DESCONECTADO",
            "t425": "NÃO CONSULTADO",
            "core": "NÃO OBSERVADO",
            "boot": "NÃO OBSERVADO",
            "links": {},
            "linkusb_b4": {},
            "linkusb_delta": {},
            "btl": None,
            "execution": "DIAGNÓSTICO",
            "iserver_exit_code": None,
            "iserver_stdout_bytes": 0,
            "iserver_stderr_bytes": 0,
            "t425_status": None,
            "t425_observed_monotonic": 0.0,
            "linkusb_observed_monotonic": 0.0,
        }
        self.event_path = Path(event_path or (Path.home() / "T425_OUTPUT" / "F296_T425_CONTROL_CENTER" / "logs" / "events.jsonl"))
        self.event_path.parent.mkdir(parents=True, exist_ok=True)
        self._stop = threading.Event()
        self._worker = threading.Thread(target=self._run, name="t425-hardware-owner", daemon=True)
        self._usb = None
        self._usb_lock_fd = None
        self._last_poll = 0.0
        self._last_cdc = 0.0
        self._last_b4 = {}
        self._last_t425 = {}
        self._seq = 0
        from iserver.iserver_runner import IServerRunner
        selected_executable, selected_sha256, selection_mode = select_iserver_executable()
        self.iserver_executable = str(selected_executable)
        self.iserver = IServerRunner(
            self.iserver_executable,
            self._on_iserver_event,
            enabled=True)
        self.emit("ISERVER_EXECUTABLE_SELECTED", "SYSTEM", "configuration", {
            "path": self.iserver_executable,
            "sha256": selected_sha256,
            "selection": selection_mode,
        })

    def start(self):
        self._worker.start()

    def close(self):
        self._stop.set()
        self.commands.put(("__stop__", None, "SYSTEM"))
        if self._worker.ident is not None:
            self._worker.join(timeout=4)
        self.iserver.stop()

    def submit(self, operation, payload=None, origin="USER"):
        job_id = "%d-%d" % (time.time_ns(), os.getpid())
        self.commands.put((operation, payload or {}, origin, job_id))
        self.emit("COMMAND_QUEUED", origin, operation, {"job_id": job_id})
        return job_id

    def emit(self, kind, origin, operation, details=None):
        with self.state_lock:
            self._seq += 1
            event = {"seq": self._seq, "timestamp": utc_now(), "origin": origin,
                     "kind": kind, "operation": operation, "details": details or {}}
            self.events.put(event)
            try:
                with self.event_path.open("a", encoding="utf-8") as f:
                    f.write(json.dumps(event, ensure_ascii=False) + "\n")
            except OSError:
                pass

    def snapshot(self):
        with self.state_lock:
            return json.loads(json.dumps(self.state))

    def _run(self):
        while not self._stop.is_set():
            try:
                cmd = self.commands.get(timeout=0.1)
            except queue.Empty:
                self._poll_if_due()
                continue
            if cmd[0] == "__stop__":
                break
            op, payload, origin, job_id = cmd
            start = time.monotonic()
            self.emit("COMMAND_STARTED", origin, op, {"job_id": job_id})
            try:
                if op == "connect_linkusb":
                    result = self._connect_linkusb()
                elif op == "disconnect_linkusb":
                    self._disconnect_linkusb()
                    result = {"connected": False}
                elif op == "query_t425":
                    result = self._query_t425()
                elif op == "snapshot_linkusb":
                    result = self._snapshot_linkusb()
                elif op == "start_iserver":
                    result = self._start_iserver(payload)
                else:
                    raise ValueError("Operação não disponível: " + op)
                classification = "PROCESS_STARTED_NOT_BOOT_CONFIRMED" if op == "start_iserver" else "SUCESSO"
                self.emit("JOB_RESULT", origin, op, {"job_id": job_id, "result": result,
                                                     "duration_ms": int((time.monotonic()-start)*1000),
                                                     "classification": classification})
            except Exception as exc:
                self.emit("JOB_RESULT", origin, op, {"job_id": job_id, "error": str(exc),
                                                     "duration_ms": int((time.monotonic()-start)*1000),
                                                     "classification": "ERRO"})
            self._poll_if_due(force=True)
        self._disconnect_linkusb()

    def _connect_linkusb(self):
        from linkusb.diagnostics import LinkUsbDiagnosticSession
        if self._usb is not None:
            return {"connected": True}
        # Shared advisory lock with the native IServer backend prevents two USB owners.
        import fcntl
        lock_path = Path("/tmp/t425-sim-linkusb0.lock")
        fd = os.open(str(lock_path), os.O_CREAT | os.O_RDWR, 0o600)
        try:
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            os.close(fd)
            raise RuntimeError("LINKUSB ocupado por outro processo (incluindo IServer)")
        self._usb_lock_fd = fd
        try:
            self._usb = LinkUsbDiagnosticSession()
            self._usb.connect()
        except Exception:
            self._usb = None
            os.close(fd)
            self._usb_lock_fd = None
            raise
        with self.state_lock:
            self.state["linkusb"] = "CONECTADO — diagnóstico B4 somente leitura"
        self.emit("DEVICE_CONNECTED", "SYSTEM", "LINKUSB", {"vid": "1209", "pid": "4250"})
        return {"connected": True, "bulk_out_used": False, "bulk_in_used": False}

    def _disconnect_linkusb(self):
        close_error = None
        if self._usb is not None:
            try:
                self._usb.close()
            except Exception as exc:
                close_error = exc
            finally:
                self._usb = None
        if self._usb_lock_fd is not None:
            import fcntl
            fcntl.flock(self._usb_lock_fd, fcntl.LOCK_UN)
            os.close(self._usb_lock_fd)
            self._usb_lock_fd = None
        with self.state_lock:
            was_connected = self.state["linkusb"] != "DESCONECTADO"
            self.state["linkusb"] = "DESCONECTADO"
        if was_connected:
            self.emit("DEVICE_DISCONNECTED", "SYSTEM", "LINKUSB", {})
        if close_error is not None:
            raise RuntimeError("Falha ao liberar sessão diagnóstica LINKUSB: %s" % close_error)

    def _snapshot_linkusb(self):
        if self._usb is None:
            raise RuntimeError("Conecte LINKUSB explicitamente")
        data = {i: self._usb.b4(i) for i in (0, 1)}
        counter_keys = ("usb_out_bytes", "host_tx_enqueued_bytes", "phy_tx_submitted_frames",
                        "ack_rx_frames", "phy_rx_data_frames", "ack_tx_submitted_frames",
                        "host_rx_enqueued_bytes", "usb_in_accepted_bytes", "malformed_words")
        delta = {}
        for i, snap in data.items():
            prev = self._last_b4.get(i)
            delta[i] = ({k: (snap[k] - prev[k]) & 0xffffffff for k in counter_keys} if prev else {k: None for k in counter_keys})
        self._last_b4 = {i: dict(v) for i, v in data.items()}
        with self.state_lock:
            self.state["links"].update({"LINKUSB%d" % i: data[i] for i in data})
            self.state["linkusb_b4"] = data
            self.state["linkusb_delta"] = delta
            self.state["linkusb_observed_monotonic"] = time.monotonic()
        self.emit("DIAGNOSTIC_SNAPSHOT", "SYSTEM", "EP0_B4", {"links": data, "delta": delta})
        return {"links": data, "delta": delta}

    def _query_t425(self):
        from t425.cdc import query_f271
        result = query_f271()
        deltas = []
        for i, link in enumerate(result.get("links", [])):
            prev = self._last_t425.get(i)
            counters = ("rx_phy_data", "ack_tx_submitted", "service_delivered", "core_rx_admitted",
                        "compat_consumed", "core_tx_accepted", "phy_tx_submitted", "ack_rx")
            deltas.append({k: ((link.get(k, 0) - prev.get(k, 0)) & 0xffffffff if prev else None) for k in counters})
        self._last_t425 = {i: dict(link) for i, link in enumerate(result.get("links", []))}
        with self.state_lock:
            self.state["t425"] = "CDC RESPONDEU"
            self.state["core"] = result.get("core1_state", "NÃO INFORMADO")
            self.state["boot"] = result.get("boot_state", "NÃO INFORMADO")
            self.state["t425_status"] = result
            self.state["t425_observed_monotonic"] = time.monotonic()
            endpoints = result.get("links", [])
            self.state["links"].update({"T425%d" % i: ep for i, ep in enumerate(endpoints)})
        self.emit("DIAGNOSTIC_SNAPSHOT", "SYSTEM", "F271?", {"status": result, "delta": deltas})
        return result

    def execution_preflight(self, btl_info=None):
        """Read-only gate. It never opens USB, reads Bulk IN, or transmits data."""
        now = time.monotonic()
        reasons = []
        with self.state_lock:
            status = self.state.get("t425_status")
            status_age = now - self.state.get("t425_observed_monotonic", 0.0)
            linkusb = self.state.get("linkusb_b4", {})
            linkusb_age = now - self.state.get("linkusb_observed_monotonic", 0.0)
            link0 = linkusb.get(0, linkusb.get("0"))
            has_usb_session = self._usb is not None
            active = self.iserver.process is not None

        if btl_info is None:
            reasons.append("Selecione um arquivo BTL válido")
        elif Path(btl_info.path).suffix.lower() != ".btl":
            reasons.append("O arquivo selecionado precisa ter extensão .btl")
        if not status or status_age > 30:
            reasons.append("Consulte F271? novamente; status ausente ou com mais de 30 s")
        else:
            if status.get("protocol") != 1:
                reasons.append("Versão de protocolo F271? não suportada")
            if status.get("startup_stage") != "RUNNING":
                reasons.append("T425 startup_stage não está RUNNING")
            if status.get("core1_state") != 2:
                reasons.append("Core1 não está RUNNING")
            if status.get("boot_state") != 2:
                reasons.append("T425 não está em WAIT_CONTROL")
            if status.get("failure_reason") is not None or status.get("failure_code") is not None:
                reasons.append("T425 reporta falha de startup")
            endpoints = status.get("links") or []
            if not endpoints or endpoints[0].get("endpoint") != 0 or not endpoints[0].get("available"):
                reasons.append("Endpoint físico Link0 do T425 não está disponível")
        if not has_usb_session or linkusb_age > 60 or not link0:
            reasons.append("Atualize EP0 B4 do LINKUSB; sessão ausente ou snapshot com mais de 60 s")
        elif not link0.get("started"):
            reasons.append("PHY Link0 do LINKUSB não está iniciada")
        else:
            if not link0.get("tx_ready") or link0.get("tx_waiting_ack"):
                reasons.append("LinkUSB Link0 TX não está pronto ou aguarda ACK")
            if link0.get("host_tx_queue_depth") != 0 or link0.get("host_rx_queue_depth") != 0:
                reasons.append("LinkUSB Link0 tem bytes pendentes nas filas")
            if link0.get("data_pending") or link0.get("physical_ack_pending"):
                reasons.append("LinkUSB Link0 tem evento físico pendente")
            if link0.get("malformed_words") != 0:
                reasons.append("LinkUSB reporta palavras físicas malformadas")
        if active:
            reasons.append("Um processo IServer já está ativo")
        return {"ready": not reasons, "reasons": reasons,
                "t425_age_seconds": None if not status else round(status_age, 3),
                "linkusb_b4_age_seconds": None if not link0 else round(linkusb_age, 3)}

    def _start_iserver(self, payload):
        from services.btl import inspect_btl
        info = inspect_btl(payload.get("path", ""))
        if info.sha256 != payload.get("sha256") or info.size != payload.get("size"):
            raise RuntimeError("O BTL mudou desde a seleção; selecione e valide o arquivo novamente")
        gate = self.execution_preflight(info)
        if not gate["ready"]:
            raise RuntimeError("Pré-checagem física reprovada: " + "; ".join(gate["reasons"]))
        # Release the diagnostic libusb handle and shared lock before IServer opens LinkIO.
        self._disconnect_linkusb()
        pid = self.iserver.start(info.path)
        with self.state_lock:
            self.state["execution"] = "ISERVER iniciado; boot ainda não confirmado"
            self.state["iserver_exit_code"] = None
        self.emit("ISERVER_PROCESS_STARTED", "USER", "Boot+Serve", {
            "pid": pid, "btl": info.name, "size": info.size, "sha256": info.sha256,
            "semantics": "processo iniciado; não prova Bulk OUT nem ACK físico"})
        return {"pid": pid, "phase": "PROCESS_STARTED_NOT_BOOT_CONFIRMED"}

    def _on_iserver_event(self, kind, text, metadata=None):
        metadata = dict(metadata or {})
        if kind == "ISERVER_EXIT":
            with self.state_lock:
                self.state["execution"] = "ISERVER encerrado (exit code %s)" % text
                self.state["iserver_exit_code"] = metadata.get("exit_code")
        elif kind in ("ISERVER_STDOUT", "ISERVER_STDERR", "LINKIO_DIAGNOSTIC"):
            with self.state_lock:
                key = "iserver_stdout_bytes" if kind.endswith("STDOUT") else "iserver_stderr_bytes"
                self.state[key] += int(metadata.get("byte_count", 0))
        operation = "LINKIO/ReadLink" if kind == "LINKIO_DIAGNOSTIC" else "process"
        self.emit(kind, "ISERVER", operation, {"text": text, **metadata})

    def _poll_if_due(self, force=False):
        now = time.monotonic()
        if self._usb is not None and (force or now - self._last_poll >= 2.0):
            self._last_poll = now
            try:
                self._snapshot_linkusb()
            except Exception as exc:
                self.emit("DIAGNOSTIC_ERROR", "SYSTEM", "EP0_B4", {"error": str(exc)})
        # CDC is explicitly user-triggered; it is not opened periodically.
