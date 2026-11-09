import base64
import codecs
import os
import subprocess
import threading
from pathlib import Path


MAX_INMOS_COMMAND_CHARS = 255  # MAX_STRING_LENGTH includes the terminating NUL.


def prepare_inmos_argv(executable, btl_path):
    """Keep INMOS RealCommandLine below its 256-byte buffer without changing cwd."""
    executable = str(executable)
    btl_path = str(btl_path)
    argv = [executable, "-sc", btl_path, "-ss", "-sl", "linkusb0"]

    def serialized_length(values):
        # Matches INMOS QuotedArg(): arguments containing spaces/tabs are quoted.
        rendered = ['"' + arg + '"' if (" " in arg or "\t" in arg) else arg
                    for arg in values]
        return len(" ".join(rendered))

    before = serialized_length(argv)
    executable_override = None
    shortened = False
    if before > MAX_INMOS_COMMAND_CHARS:
        argv[0] = Path(executable).name
        executable_override = executable
        shortened = True
    after = serialized_length(argv)
    if after > MAX_INMOS_COMMAND_CHARS:
        raise ValueError("IServer argv continua acima do limite INMOS de 255 caracteres (%d)" % after)
    return {
        "argv": argv,
        "executable": executable_override,
        "original_length": before,
        "final_length": after,
        "shortened": shortened,
        "cwd": os.getcwd(),
    }


class IServerRunner:
    def __init__(self, executable, event_callback, enabled=False):
        self.executable = executable
        self.event_callback = event_callback
        self.enabled = enabled
        self.process = None
        self._threads = []
        self._reader_threads = []
        self._lock = threading.RLock()

    def start(self, btl_path):
        if not self.enabled:
            raise RuntimeError("Execução de hardware está desabilitada nesta versão F299.1")
        with self._lock:
            if self.process is not None:
                raise RuntimeError("IServer já está em execução")
            if not Path(self.executable).is_file():
                raise RuntimeError("Executável IServer ausente: " + self.executable)
            if not Path(btl_path).is_file():
                raise RuntimeError("Arquivo BTL ausente: " + btl_path)
            launch = prepare_inmos_argv(self.executable, btl_path)
            if launch["shortened"]:
                self.event_callback("ISERVER_COMMAND_LINE_SHORTENED", "", {
                    "executable": self.executable,
                    "argv0": launch["argv"][0],
                    "btl_argument": launch["argv"][2],
                    "original_length": launch["original_length"],
                    "final_length": launch["final_length"],
                    "cwd": launch["cwd"],
                    "semantics": "argv0 encurtado; executável real e CWD preservados",
                })
            self.process = subprocess.Popen(
                launch["argv"], executable=launch["executable"],
                stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                bufsize=0, close_fds=True,
            )
            process = self.process
            self._reader_threads = []
            for pipe, name in ((process.stdout, "stdout"), (process.stderr, "stderr")):
                thread = threading.Thread(
                    target=self._forward, args=(pipe, name),
                    name="iserver-%s-reader" % name, daemon=True,
                )
                thread.start()
                self._reader_threads.append(thread)
            watcher = threading.Thread(
                target=self._watch, args=(process, tuple(self._reader_threads)),
                name="iserver-process-watch", daemon=True,
            )
            watcher.start()
            self._threads = list(self._reader_threads) + [watcher]
            return process.pid

    def _watch(self, process, readers):
        code = process.wait()
        # Both pipes are drained concurrently; wait for their EOF before emitting exit.
        for thread in readers:
            thread.join()
        for pipe in (process.stdout, process.stderr):
            if pipe is not None:
                pipe.close()
        with self._lock:
            if self.process is process:
                self.process = None
        self.event_callback("ISERVER_EXIT", str(code), {"exit_code": code})

    def _forward(self, stream, name):
        if name == "stderr":
            self._forward_stderr(stream)
            return
        decoder = codecs.getincrementaldecoder("utf-8")("replace")
        offset = 0
        fd = stream.fileno()
        while True:
            try:
                raw = os.read(fd, 4096)
            except OSError as exc:
                self.event_callback("ISERVER_STREAM_ERROR", str(exc), {"stream": name, "offset": offset})
                return
            if not raw:
                break
            text = decoder.decode(raw, final=False)
            self.event_callback(
                "ISERVER_" + name.upper(), text,
                {"stream": name, "byte_offset": offset,
                 "byte_count": len(raw), "raw_base64": base64.b64encode(raw).decode("ascii")},
            )
            offset += len(raw)
        tail = decoder.decode(b"", final=True)
        if tail:
            self.event_callback("ISERVER_" + name.upper(), tail,
                                {"stream": name, "byte_offset": offset, "byte_count": 0,
                                 "raw_base64": ""})
        self.event_callback("ISERVER_STREAM_EOF", "", {"stream": name, "byte_offset": offset})

    def _forward_stderr(self, stream):
        prefix = b"[LINKIO_DIAG]"
        pending = bytearray()
        offset = 0
        record_offset = 0
        current_is_diag = None
        fd = stream.fileno()

        def emit_record(raw, at, complete):
            is_diag = raw.startswith(prefix) if current_is_diag is None else current_is_diag
            kind = "LINKIO_DIAGNOSTIC" if is_diag else "ISERVER_STDERR"
            metadata = {"stream": "stderr", "byte_offset": at, "byte_count": len(raw),
                        "raw_base64": base64.b64encode(raw).decode("ascii"),
                        "line_complete": bool(complete)}
            if is_diag:
                metadata["classification"] = "TRANSPORT_DIAGNOSTIC"
            self.event_callback(kind, raw.decode("utf-8", "replace"), metadata)

        while True:
            try:
                raw = os.read(fd, 4096)
            except OSError as exc:
                self.event_callback("ISERVER_STREAM_ERROR", str(exc),
                                    {"stream": "stderr", "offset": offset})
                return
            if not raw:
                break
            pending.extend(raw)
            offset += len(raw)
            while True:
                newline = pending.find(b"\n")
                if newline >= 0:
                    record = bytes(pending[:newline + 1])
                    emit_record(record, record_offset, True)
                    del pending[:newline + 1]
                    record_offset += len(record)
                    current_is_diag = None
                    continue
                if current_is_diag is None:
                    if len(pending) >= len(prefix):
                        current_is_diag = pending.startswith(prefix)
                    elif not prefix.startswith(bytes(pending)):
                        current_is_diag = False
                if len(pending) >= 8192:
                    record = bytes(pending[:4096])
                    emit_record(record, record_offset, False)
                    del pending[:4096]
                    record_offset += len(record)
                    continue
                break

        if pending:
            emit_record(bytes(pending), record_offset, False)
            record_offset += len(pending)
        self.event_callback("ISERVER_STREAM_EOF", "", {"stream": "stderr", "byte_offset": offset})

    def stop(self):
        with self._lock:
            process = self.process
        if process is None:
            return
        # Terminating this subprocess stops only the host IServer, not the T425 guest.
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        for thread in tuple(self._reader_threads):
            if thread is not threading.current_thread():
                thread.join(timeout=3)
        with self._lock:
            if self.process is process:
                self.process = None
