import base64
import json
import tempfile
import threading
import time
import tkinter as tk
import unittest
from pathlib import Path
from unittest import mock

from gui.occam_console import OccamConsole
from iserver.iserver_runner import IServerRunner


class F2991ConsoleTests(unittest.TestCase):
    def test_visible_layout_maps_main_panels_without_hardware(self):
        import gui.app as app_module
        old_support = app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as tmp:
            app_module.APP_SUPPORT = Path(tmp) / "support"
            root = tk.Tk()
            app = app_module.ControlCenter(root)
            root.update_idletasks()
            root.update()
            self.assertTrue(root.winfo_viewable())
            self.assertTrue(app.notebook.winfo_ismapped())
            self.assertGreater(app.console.listbox.winfo_width(), 300)
            self.assertGreater(app.console.listbox.winfo_height(), 50)
            self.assertLessEqual(root.winfo_rooty() + root.winfo_height(), root.winfo_screenheight() - 70)
            self.assertEqual([app.notebook.tab(tab, "text") for tab in app.notebook.tabs()],
                             ["Console OCCAM (IServer)", "Eventos / Diagnóstico", "Snapshots F271/B4", "Logs"])
            self.assertTrue(all(label.winfo_ismapped() for label in app.device_labels.values()))
            app.close()
            app_module.APP_SUPPORT = old_support

    def test_console_renders_unicode_partial_lines_copy_and_save(self):
        root = tk.Tk(); root.withdraw()
        console = OccamConsole(root)
        console.append_text("stdout", "Hello ")
        console.append_text("stdout", "Mundo — ação ✓\nparte")
        console.append_text("stderr", "diagnóstico")
        root.update_idletasks()
        rows = console.listbox.get(0, "end")
        self.assertEqual(rows, ("[stdout] Hello Mundo — ação ✓", "[stdout] parte", "[stderr] diagnóstico"))
        copied = console.copy_all()
        self.assertEqual(copied.splitlines(), list(rows))
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "console.txt"
            console.save(out)
            self.assertEqual(out.read_text(encoding="utf-8").splitlines(), list(rows))
        console.flush_stream("stdout")
        self.assertEqual(console.listbox.get(1), "[stdout] parte")
        root.destroy()

    def test_console_handles_large_volume_and_remains_event_driven(self):
        root = tk.Tk(); root.withdraw()
        console = OccamConsole(root)
        tick = []
        root.after(0, lambda: tick.append(True))
        console.append_text("stdout", "linha-000\n" + "".join("linha-%05d\n" % i for i in range(1, 3000)))
        root.update()
        self.assertEqual(console.listbox.size(), 3000)
        self.assertEqual(console.listbox.get("end"), "[stdout] linha-02999")
        self.assertTrue(tick)
        root.destroy()

    def test_runner_preserves_partial_output_bytes_per_stream_and_exit_code(self):
        events = []
        done = threading.Event()
        lock = threading.Lock()
        def callback(kind, text, metadata=None):
            with lock:
                events.append((kind, text, metadata or {}))
            if kind == "ISERVER_EXIT":
                done.set()
        with tempfile.TemporaryDirectory() as tmp:
            script = Path(tmp) / "fake-iserver"
            script.write_text("#!/bin/sh\nprintf 'Olá '; sleep 0.05; printf 'T425 ✓\\n'; printf 'erro-parcial' >&2; exit 7\n", encoding="utf-8")
            script.chmod(0o755)
            btl = Path(tmp) / "demo.btl"; btl.write_bytes(b"fixture")
            runner = IServerRunner(str(script), callback, enabled=True)
            runner.start(str(btl))
            self.assertTrue(done.wait(5))
            stdout = "".join(text for kind, text, _ in events if kind == "ISERVER_STDOUT")
            stderr = "".join(text for kind, text, _ in events if kind == "ISERVER_STDERR")
            self.assertEqual(stdout, "Olá T425 ✓\n")
            self.assertEqual(stderr, "erro-parcial")
            self.assertTrue(any(kind == "ISERVER_EXIT" and meta.get("exit_code") == 7 for kind, _, meta in events))
            for stream_name, kind in (("stdout", "ISERVER_STDOUT"), ("stderr", "ISERVER_STDERR")):
                chunks = [meta for event_kind, _, meta in events if event_kind == kind and meta.get("byte_count")]
                raw = b"".join(base64.b64decode(meta["raw_base64"]) for meta in chunks)
                expected = "Olá T425 ✓\n".encode() if stream_name == "stdout" else b"erro-parcial"
                self.assertEqual(raw, expected)
                self.assertEqual([meta["byte_offset"] for meta in chunks],
                                 [sum(x["byte_count"] for x in chunks[:i]) for i in range(len(chunks))])

    def test_gui_routes_only_real_iserver_output_to_occam_console(self):
        import gui.app as app_module
        old_support = app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as tmp:
            app_module.APP_SUPPORT = Path(tmp) / "support"
            root = tk.Tk(); root.withdraw()
            app = app_module.ControlCenter(root)
            start = {"seq": 1, "timestamp": "t0", "origin": "ISERVER", "operation": "process",
                     "kind": "ISERVER_PROCESS_STARTED", "details": {"pid": 10}}
            app._display_event(start)
            self.assertEqual(app.console.listbox.size(), 0)
            payload = "Saída OCCAM recebida\n".encode("utf-8")
            app._display_event({"seq": 2, "timestamp": "t1", "origin": "ISERVER", "operation": "process",
                                "kind": "ISERVER_STDOUT", "details": {"stream": "stdout", "text": "Saída OCCAM recebida\n",
                                "byte_count": len(payload), "raw_base64": base64.b64encode(payload).decode()}})
            app._display_event({"seq": 3, "timestamp": "t2", "origin": "ISERVER", "operation": "process",
                                "kind": "ISERVER_STDERR", "details": {"stream": "stderr", "text": "warning\n",
                                "byte_count": 8, "raw_base64": base64.b64encode(b"warning\n").decode()}})
            self.assertEqual(app.console.listbox.get(0, "end"), ("[stdout] Saída OCCAM recebida", "[stderr] warning"))
            self.assertEqual(app.summary_labels["boot"].cget("text"), "Submetido pelo IServer; não confirmado")
            self.assertIn("IServer", app.summary_labels["program"].cget("text"))
            self.assertEqual(str(app.run_button["state"]), "disabled")
            self.assertEqual(str(app.stop_button["state"]), "disabled")
            self.assertEqual(str(app.reset_button["state"]), "disabled")
            app.close()
            app_module.APP_SUPPORT = old_support

    def test_close_window_during_active_subprocess_terminates_host_only(self):
        import gui.app as app_module
        old_support = app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as tmp:
            app_module.APP_SUPPORT = Path(tmp) / "support"
            script = Path(tmp) / "long-iserver"
            script.write_text("#!/bin/sh\nsleep 10\n", encoding="utf-8"); script.chmod(0o755)
            btl = Path(tmp) / "x.btl"; btl.write_bytes(b"x")
            root = tk.Tk(); root.withdraw()
            app = app_module.ControlCenter(root)
            app.controller.iserver = IServerRunner(str(script), app.controller._on_iserver_event, enabled=True)
            app.controller.iserver.start(str(btl))
            proc = app.controller.iserver.process
            with mock.patch.object(app_module.messagebox, "askyesno", return_value=True):
                app.close()
            self.assertIsNotNone(proc.poll())
            with self.assertRaises(tk.TclError):
                root.winfo_exists()
            app_module.APP_SUPPORT = old_support


if __name__ == "__main__":
    unittest.main()
