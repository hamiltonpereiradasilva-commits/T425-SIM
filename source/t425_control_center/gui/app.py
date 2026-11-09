import json
import queue
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from gui.occam_console import OccamConsole
from services.btl import inspect_btl
from services.controller import MonitorController

APP_SUPPORT = Path.home() / "Library" / "Application Support" / "T425-SIM Control Center F299.1"
APP_LOG_PATH = Path.home() / "T425_OUTPUT" / "F299_1" / "logs" / "events.jsonl"


class ControlCenter:
    def __init__(self, root):
        self.root = root
        self.root.title("T425-SIM Control Center — F299.1")
        screen_width = int(self.root.winfo_screenwidth())
        screen_height = int(self.root.winfo_screenheight())
        width = min(1120, max(980, screen_width - 40))
        # Tk geometry excludes the decorated title bar; reserve extra space for
        # the macOS Dock after accounting for the native window decorations.
        height = min(740, max(500, screen_height - 200))
        x = max(10, (screen_width - width) // 2)
        self.root.geometry("%dx%d+%d+0" % (width, height, x))
        self.root.minsize(min(980, width), min(500, height))
        self.controller = MonitorController(event_path=APP_LOG_PATH)
        self.controller.start()
        self.btl = None
        self.last_seq = 0
        self.execution_armed = tk.BooleanVar(value=False)
        self._log_items = []
        self._console_output_seen = False
        self._build()
        self.root.after(100, self._pump)
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self._log("Control Center pronto; nenhum dispositivo foi aberto nem byte transmitido.")
        self._refresh_execution_gate()

    def _build(self):
        header = ttk.Frame(self.root, padding=(10, 8))
        header.pack(fill="x")
        ttk.Label(header, text="T425-SIM CONTROL CENTER", font=("TkDefaultFont", 16, "bold")).pack(side="left")
        ttk.Label(header, text="F299.1 · console de saída real do processo IServer",
                  foreground="#365b85").pack(side="right")

        devices = ttk.LabelFrame(self.root, text="Estado dos dispositivos", padding=8)
        devices.pack(fill="x", padx=10, pady=(0, 5))
        self.device_labels = {}
        for key, title in (("t425", "T425-SIM"), ("linkusb", "LINKUSB"),
                           ("core", "Core1 / guest"), ("boot", "Boot")):
            box = ttk.Frame(devices, padding=(6, 2))
            box.pack(side="left", fill="x", expand=True)
            ttk.Label(box, text=title, font=("TkDefaultFont", 9, "bold")).pack(anchor="w")
            self.device_labels[key] = ttk.Label(box, text="Não consultado")
            self.device_labels[key].pack(anchor="w")
        actions = ttk.Frame(devices)
        actions.pack(side="right")
        ttk.Button(actions, text="Conectar LINKUSB", command=lambda: self.controller.submit("connect_linkusb")).pack(side="left", padx=2)
        ttk.Button(actions, text="Desconectar", command=lambda: self.controller.submit("disconnect_linkusb")).pack(side="left", padx=2)
        ttk.Button(actions, text="Consultar T425 F271?", command=lambda: self.controller.submit("query_t425")).pack(side="left", padx=2)

        program = ttk.LabelFrame(self.root, text="Programa OCCAM (.BTL)", padding=8)
        program.pack(fill="x", padx=10, pady=4)
        row = ttk.Frame(program)
        row.pack(fill="x")
        ttk.Button(row, text="Selecionar BTL…", command=self.select_btl).pack(side="left")
        self.btl_label = ttk.Label(row, text="Nenhum arquivo selecionado")
        self.btl_label.pack(side="left", padx=9)
        self.load_button = ttk.Button(row, text="Load + Serve", state="disabled", command=self.load_selected_btl)
        self.load_button.pack(side="right", padx=2)
        self.run_button = ttk.Button(row, text="Run", state="disabled")
        self.run_button.pack(side="right", padx=2)
        self.stop_button = ttk.Button(row, text="Stop", state="disabled")
        self.stop_button.pack(side="right", padx=2)
        self.reset_button = ttk.Button(row, text="Reset", state="disabled")
        self.reset_button.pack(side="right", padx=2)
        gate_row = ttk.Frame(program)
        gate_row.pack(fill="x", pady=(6, 0))
        self.arm_check = ttk.Checkbutton(gate_row, text="Armar execução física após pré-checagens",
                                         variable=self.execution_armed, command=self._refresh_execution_gate,
                                         state="disabled")
        self.arm_check.pack(side="left")
        self.gate_label = ttk.Label(gate_row, text="Selecione BTL e consulte T425 F271? e LINKUSB B4 Link0.")
        self.gate_label.pack(side="left", padx=8)
        self.progress = ttk.Progressbar(program, mode="determinate", maximum=100, value=0)
        self.progress.pack(fill="x", pady=(7, 2))
        self.btl_detail = ttk.Label(program, text="O IServer inicia Boot e Serve; progresso por byte não é exposto.")
        self.btl_detail.pack(anchor="w")

        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill="both", expand=True, padx=10, pady=4)
        self.console_tab = ttk.Frame(self.notebook, padding=7)
        self.events_tab = ttk.Frame(self.notebook, padding=7)
        self.snapshots_tab = ttk.Frame(self.notebook, padding=7)
        self.logs_tab = ttk.Frame(self.notebook, padding=7)
        self.notebook.add(self.console_tab, text="Console OCCAM (IServer)")
        self.notebook.add(self.events_tab, text="Eventos / Diagnóstico")
        self.notebook.add(self.snapshots_tab, text="Snapshots F271/B4")
        self.notebook.add(self.logs_tab, text="Logs")
        self._build_console_tab()
        self._build_events_tab()
        self._build_snapshots_tab()
        self._build_logs_tab()

    def _build_console_tab(self):
        self._build_execution_summary(self.console_tab)
        self.console = OccamConsole(self.console_tab)
        self.console.pack(fill="both", expand=True)

    def _build_events_tab(self):
        columns = ("timestamp", "origin", "operation", "kind", "details")
        self.event_tree = ttk.Treeview(self.events_tab, columns=columns, show="headings", height=8)
        for col, title, width in zip(columns, ("Timestamp", "Origem", "Operação", "Evento", "Detalhes"),
                                     (190, 100, 145, 190, 590)):
            self.event_tree.heading(col, text=title)
            self.event_tree.column(col, width=width, anchor="w")
        ybar = ttk.Scrollbar(self.events_tab, orient="vertical", command=self.event_tree.yview)
        xbar = ttk.Scrollbar(self.events_tab, orient="horizontal", command=self.event_tree.xview)
        self.event_tree.configure(yscrollcommand=ybar.set, xscrollcommand=xbar.set)
        self.event_tree.pack(side="top", fill="both", expand=True)
        ybar.pack(side="right", fill="y")
        xbar.pack(side="bottom", fill="x")

    def _build_snapshots_tab(self):
        links = ttk.LabelFrame(self.snapshots_tab, text="Contadores dos links físicos — dados medidos/reportados", padding=6)
        links.pack(fill="x", pady=(0, 6))
        cols = ("endpoint", "state", "usb_out", "usb_in", "tx_data", "rx_data", "ack_tx", "ack_rx", "queues", "errors")
        heads = ("Endpoint", "Estado", "USB OUT", "USB IN", "DATA TX", "DATA RX", "ACK TX", "ACK RX", "Filas TX/RX", "Erros")
        widths = (120, 145, 78, 78, 82, 82, 82, 82, 90, 72)
        self.tree = ttk.Treeview(links, columns=cols, show="headings", height=6)
        for col, title, width in zip(cols, heads, widths):
            self.tree.heading(col, text=title)
            self.tree.column(col, width=width, anchor="center")
        self.tree.pack(fill="x")
        self._link_rows = {}
        for name in ("T425 Link0", "T425 Link1", "T425 Link2", "T425 Link3", "LINKUSB Link0", "LINKUSB Link1"):
            self._link_rows[name] = self.tree.insert("", "end", values=(name, "NÃO CONSULTADO", "—", "—", "—", "—", "—", "—", "—", "—"))
        toolbar = ttk.Frame(self.snapshots_tab)
        toolbar.pack(fill="x")
        ttk.Button(toolbar, text="Consultar LINKUSB B4", command=lambda: self.controller.submit("snapshot_linkusb")).pack(side="left")
        ttk.Button(toolbar, text="Exportar snapshot…", command=self.export_snapshot).pack(side="left", padx=4)
        ttk.Label(toolbar, text="Snapshots efetivamente recebidos; campos ausentes não são inferidos.").pack(side="left")
        self.snapshot_list = tk.Listbox(toolbar.master, height=8, activestyle="none", exportselection=False,
                                        font=("Menlo", 10), background="#f5f7fa")
        ybar = ttk.Scrollbar(toolbar.master, orient="vertical", command=self.snapshot_list.yview)
        xbar = ttk.Scrollbar(toolbar.master, orient="horizontal", command=self.snapshot_list.xview)
        self.snapshot_list.configure(yscrollcommand=ybar.set, xscrollcommand=xbar.set)
        self.snapshot_list.pack(side="left", fill="both", expand=True, pady=(6, 0))
        ybar.pack(side="right", fill="y", pady=(6, 0))
        xbar.pack(side="bottom", fill="x")

    def _build_logs_tab(self):
        toolbar = ttk.Frame(self.logs_tab)
        toolbar.pack(fill="x")
        ttk.Button(toolbar, text="Salvar logs TXT…", command=self.save_logs).pack(side="left")
        self.log_list = tk.Listbox(self.logs_tab, height=8, activestyle="none", exportselection=False,
                                   font=("Menlo", 9), background="#f5f7fa")
        ybar = ttk.Scrollbar(self.logs_tab, orient="vertical", command=self.log_list.yview)
        xbar = ttk.Scrollbar(self.logs_tab, orient="horizontal", command=self.log_list.xview)
        self.log_list.configure(yscrollcommand=ybar.set, xscrollcommand=xbar.set)
        self.log_list.pack(side="left", fill="both", expand=True, pady=(6, 0))
        ybar.pack(side="right", fill="y", pady=(6, 0))
        xbar.pack(side="bottom", fill="x")

    def _build_execution_summary(self, parent):
        frame = ttk.LabelFrame(parent, text="Resumo da execução — estados separados", padding=6)
        frame.pack(fill="x", pady=(0, 5))
        self.summary_labels = {}
        for key, title, initial in (
            ("transport", "Transporte", "Aguardando observação"),
            ("boot", "Bootstrap / boot", "Não confirmado"),
            ("iserver", "IServer", "Inativo"),
            ("program", "Saída do programa", "Nenhuma saída recebida"),
        ):
            cell = ttk.Frame(frame, padding=(5, 0))
            cell.pack(side="left", fill="x", expand=True)
            ttk.Label(cell, text=title, font=("TkDefaultFont", 8, "bold")).pack(anchor="w")
            label = ttk.Label(cell, text=initial, wraplength=280)
            label.pack(anchor="w")
            self.summary_labels[key] = label

    def select_btl(self):
        path = filedialog.askopenfilename(title="Selecionar programa OCCAM BTL",
                                          filetypes=[("Bootable Transputer Link", "*.btl"), ("Todos os arquivos", "*")])
        if not path:
            return
        try:
            self.btl = inspect_btl(path)
        except Exception as exc:
            messagebox.showerror("BTL inválido", str(exc))
            return
        self.btl_label.configure(text=self.btl.name)
        self.btl_detail.configure(text="Tamanho: %d bytes | SHA-256: %s | primeiro byte: 0x%02X | %s" %
                                  (self.btl.size, self.btl.sha256, self.btl.first_byte,
                                   "Hello World F72 validado" if self.btl.expected_hello else "BTL selecionado"))
        self.progress.configure(value=0)
        self._save_config()
        self._log("BTL validado localmente; nenhum carregamento iniciado.")
        self._refresh_execution_gate()

    def _load_config(self):
        path = APP_SUPPORT / "config.json"
        try:
            return json.loads(path.read_text(encoding="utf-8"))
        except Exception:
            return {}

    def _save_config(self):
        APP_SUPPORT.mkdir(parents=True, exist_ok=True)
        path = APP_SUPPORT / "config.json"
        safe = {"last_btl": self.btl.path if self.btl else self._load_config().get("last_btl")}
        path.write_text(json.dumps(safe, indent=2), encoding="utf-8")

    def _refresh_execution_gate(self):
        gate = self.controller.execution_preflight(self.btl)
        ready = gate["ready"]
        self.arm_check.configure(state="normal" if ready else "disabled")
        if not ready and self.execution_armed.get():
            self.execution_armed.set(False)
        self.load_button.configure(state="normal" if ready and self.execution_armed.get() else "disabled")
        if ready:
            self.gate_label.configure(text="PRÉ-CHECAGEM PASSOU — Load + Serve requer armação e confirmação explícita.")
        else:
            self.gate_label.configure(text="BLOQUEADO: " + (gate["reasons"][0] if gate["reasons"] else "Pronto"))

    def load_selected_btl(self):
        if self.btl is None:
            self._log("Load bloqueado: selecione um BTL primeiro.")
            return
        gate = self.controller.execution_preflight(self.btl)
        if not gate["ready"] or not self.execution_armed.get():
            self._refresh_execution_gate()
            self._log("Load bloqueado: pré-checagem/armação inválida — " + "; ".join(gate["reasons"]))
            return
        try:
            current = inspect_btl(self.btl.path)
        except Exception as exc:
            messagebox.showerror("BTL indisponível", str(exc))
            self._refresh_execution_gate()
            return
        if current.sha256 != self.btl.sha256 or current.size != self.btl.size:
            messagebox.showerror("BTL alterado", "O arquivo mudou após a seleção. Selecione-o novamente.")
            self._refresh_execution_gate()
            return
        prompt = ("Transmitir o BTL pelo LINKUSB Link0 e iniciar Boot + Serve original do IServer?\n\n"
                  "%s\n%d bytes\nSHA-256: %s\n\n"
                  "Bulk OUT aceito não basta: o LinkIO só confirma progresso após ACK_RX observado. "
                  "O início do processo não confirma boot nem execução OCCAM. O aplicativo não executa reset." %
                  (current.name, current.size, current.sha256))
        if not messagebox.askyesno("Confirmar Boot + Serve físico", prompt, icon="warning"):
            self._log("Boot + Serve cancelado pelo operador; nenhum dado enviado.")
            return
        self.execution_armed.set(False)
        self.load_button.configure(state="disabled")
        self.summary_labels["iserver"].configure(text="Processo solicitado; boot ainda não confirmado")
        self.summary_labels["boot"].configure(text="Aguardando confirmação do transporte/CORE")
        self.btl_detail.configure(text="Início do IServer não confirma transferência nem execução do programa.")
        self.controller.submit("start_iserver", {"path": current.path, "sha256": current.sha256, "size": current.size}, origin="USER")

    def _pump(self):
        for _ in range(300):
            try:
                event = self.controller.events.get_nowait()
            except queue.Empty:
                break
            self._display_event(event)
        state = self.controller.snapshot()
        self.device_labels["linkusb"].configure(text=state["linkusb"])
        execution = state.get("execution")
        if execution and execution != "DIAGNÓSTICO":
            self.summary_labels["iserver"].configure(text=execution)
        self._refresh_execution_gate()
        self.root.after(100, self._pump)

    def _display_event(self, event):
        self.last_seq = event["seq"]
        kind = event["kind"]
        details = event.get("details", {})
        encoded = json.dumps(event, ensure_ascii=False, separators=(",", ":"))
        self._log_items.append(encoded)
        self.log_list.insert("end", encoded)
        self.log_list.see("end")
        detail_text = json.dumps(details, ensure_ascii=False, separators=(",", ":"))
        self.event_tree.insert("", "end", values=(event.get("timestamp", ""), event.get("origin", ""),
                                                     event.get("operation", ""), kind, detail_text))
        self.event_tree.yview_moveto(1.0)
        self._log("[%s] %s · %s · %s" % (event.get("timestamp", ""), event.get("origin", ""),
                                           event.get("operation", ""), kind))

        if kind in ("ISERVER_STDOUT", "ISERVER_STDERR"):
            stream = "stdout" if kind.endswith("STDOUT") else "stderr"
            self.console.append_text(stream, details.get("text", ""))
            if stream == "stdout" and details.get("byte_count", 0):
                self._console_output_seen = True
                total = self.controller.snapshot().get("iserver_stdout_bytes", 0)
                self.summary_labels["program"].configure(
                    text="%d bytes stdout recebidos do IServer; atribuição ao programa não inferida" % total)
            elif stream == "stderr" and details.get("byte_count", 0):
                total = self.controller.snapshot().get("iserver_stderr_bytes", 0)
                self.summary_labels["iserver"].configure(text="IServer stderr: %d bytes recebidos" % total)
        elif kind == "ISERVER_STREAM_EOF":
            self.console.flush_stream(details.get("stream", "stdout"))

        if kind == "ISERVER_PROCESS_STARTED":
            self.summary_labels["iserver"].configure(text="Processo iniciado; Boot/Serve ainda não confirmados")
            self.summary_labels["transport"].configure(text="Aguardando progresso LinkIO/ACK observado")
            self.summary_labels["boot"].configure(text="Submetido pelo IServer; não confirmado")
            self.summary_labels["program"].configure(text="Nenhuma saída recebida")
        elif kind == "ISERVER_EXIT":
            code = details.get("exit_code", details.get("text", "desconhecido"))
            self.summary_labels["iserver"].configure(text="Processo encerrado; código %s" % code)
            if self._console_output_seen:
                self.summary_labels["program"].configure(
                    text="Texto stdout recebido; sucesso OCCAM não inferido do exit code isoladamente")
            else:
                self.summary_labels["program"].configure(text="Nenhuma saída stdout recebida")
        elif kind == "JOB_RESULT" and event.get("operation") == "start_iserver":
            result = details.get("result", {})
            if result.get("phase") == "PROCESS_STARTED_NOT_BOOT_CONFIRMED":
                self.summary_labels["iserver"].configure(text="Subprocesso iniciado; boot não confirmado")
        elif kind == "DIAGNOSTIC_SNAPSHOT":
            snapshot = {"timestamp": event.get("timestamp"), "operation": event.get("operation"), "details": details}
            for line in json.dumps(snapshot, ensure_ascii=False, indent=2).splitlines():
                self.snapshot_list.insert("end", line)
            self.snapshot_list.insert("end", "" )
            self.snapshot_list.see("end")
            if event.get("operation") == "EP0_B4":
                for index, snap in details.get("links", {}).items():
                    delta = details.get("delta", {}).get(index, {})
                    self._update_link("LINKUSB Link%d" % int(index), snap, "B4", delta)
                self.summary_labels["transport"].configure(text="Snapshot B4 recebido; não é ACK por si só")
            elif event.get("operation") == "F271?":
                status = details.get("status", {})
                self.device_labels["t425"].configure(text="CDC respondeu")
                self.device_labels["core"].configure(text={0: "INITIALIZING", 1: "RESOURCES_READY", 2: "RUNNING", 3: "FAILED", 4: "STOPPED"}.get(status.get("core1_state"), "NÃO INFORMADO"))
                self.device_labels["boot"].configure(text={0: "RESET", 1: "DECIDE", 2: "WAIT_CONTROL", 3: "POKE_ADDRESS", 4: "POKE_DATA", 5: "PEEK_ADDRESS", 6: "PEEK_OUTPUT", 7: "LOAD", 8: "RUNNING"}.get(status.get("boot_state"), "NÃO INFORMADO"))
                for index, endpoint in enumerate(status.get("links", [])):
                    self._update_link("T425 Link%d" % index, endpoint, "CDC", (details.get("delta") or [{}] * 4)[index])
                self.summary_labels["boot"].configure(text="boot_state=%s; não comprova execução concluída" % status.get("boot_state", "não informado"))
        elif kind == "DEVICE_CONNECTED":
            self.device_labels["linkusb"].configure(text="CONECTADO")

    def _update_link(self, name, data, source, delta=None):
        if name not in self._link_rows:
            return
        delta = delta or {}
        if source == "B4":
            def fmt(key):
                value = data.get(key, "—")
                change = delta.get(key)
                return str(value) if change is None or value == "—" else "%s (+%s)" % (value, change)
            values = (name, "STARTED" if data.get("started") else "STOPPED",
                      fmt("usb_out_bytes"), fmt("usb_in_accepted_bytes"), fmt("phy_tx_submitted_frames"),
                      fmt("phy_rx_data_frames"), fmt("ack_tx_submitted_frames"), fmt("ack_rx_frames"),
                      "%s/%s" % (data.get("host_tx_queue_depth", "—"), data.get("host_rx_queue_depth", "—")),
                      fmt("malformed_words"))
        else:
            def fmt(key):
                value = data.get(key, "—")
                change = delta.get(key)
                return str(value) if change is None or value == "—" else "%s (+%s)" % (value, change)
            values = (name, str(data.get("state", "AVAILABLE" if data.get("available") else "UNAVAILABLE")),
                      "—", "—", fmt("phy_tx_submitted"), fmt("rx_phy_data"), fmt("ack_tx_submitted"),
                      fmt("ack_rx"), "—", "—")
        self.tree.item(self._link_rows[name], values=values)

    def _log(self, text):
        self._log_items.append(text)
        self.log_list.insert("end", text)
        self.log_list.see("end")

    def clear_console(self):
        self.console.clear()

    def export_snapshot(self):
        path = filedialog.asksaveasfilename(defaultextension=".txt", filetypes=[("Texto", "*.txt")])
        if path:
            Path(path).write_text("T425-SIM diagnostic snapshot\nSequence: %d\n%s\n" %
                                  (self.last_seq, json.dumps(self.controller.snapshot(), ensure_ascii=False, indent=2)),
                                  encoding="utf-8")

    def save_logs(self):
        path = filedialog.asksaveasfilename(defaultextension=".txt", filetypes=[("Texto", "*.txt")])
        if path:
            Path(path).write_text("\n".join(self._log_items) + "\n", encoding="utf-8")

    def close(self):
        if self.controller.iserver.process is not None:
            if not messagebox.askyesno("IServer em execução",
                    "Fechar encerra apenas o processo host do IServer; não para nem reseta o guest. "
                    "O alvo pode permanecer parcial. Fechar mesmo assim?", icon="warning"):
                return
        self.controller.close()
        self._save_config()
        self.root.destroy()
