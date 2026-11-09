import base64
import os
import tempfile
import threading
import time
import tkinter as tk
import unittest
from pathlib import Path

from iserver.iserver_runner import IServerRunner
from services.controller import MonitorController


class F3041LinkioDiagnosticTests(unittest.TestCase):
    def test_stderr_linkio_lines_are_separated_from_other_stderr(self):
        read_fd, write_fd = os.pipe()
        stream = os.fdopen(read_fd, "rb", buffering=0)
        events = []
        thread = threading.Thread(target=IServerRunner("unused", lambda *args: events.append(args))._forward_stderr,
                                  args=(stream,))
        thread.start()
        os.write(write_fd, b"[LINKIO_DI")
        time.sleep(0.03)
        os.write(write_fd, b"AG] seq=41 event=summary reason=COUNT_COMPLETE\n")
        os.write(write_fd, b"iserver warning\n")
        os.close(write_fd)
        thread.join(timeout=3)
        stream.close()
        self.assertFalse(thread.is_alive())
        diag = [e for e in events if e[0] == "LINKIO_DIAGNOSTIC"]
        ordinary = [e for e in events if e[0] == "ISERVER_STDERR"]
        self.assertEqual(len(diag), 1)
        self.assertEqual(diag[0][2]["classification"], "TRANSPORT_DIAGNOSTIC")
        self.assertTrue(diag[0][1].startswith("[LINKIO_DIAG] seq=41"))
        self.assertEqual(base64.b64decode(diag[0][2]["raw_base64"]),
                         b"[LINKIO_DIAG] seq=41 event=summary reason=COUNT_COMPLETE\n")
        self.assertEqual("".join(e[1] for e in ordinary), "iserver warning\n")
        records = sorted(diag + ordinary, key=lambda e: e[2]["byte_offset"])
        self.assertEqual([e[2]["byte_offset"] for e in records],
                         [0, len(base64.b64decode(diag[0][2]["raw_base64"]))])
        self.assertEqual(events[-1][0], "ISERVER_STREAM_EOF")

    def test_controller_counts_diag_as_stderr_but_marks_transport_operation(self):
        with tempfile.TemporaryDirectory() as tmp:
            controller = MonitorController(event_path=Path(tmp) / "events.jsonl")
            text = "[LINKIO_DIAG] seq=2 event=summary reason=GLOBAL_DEADLINE\n"
            controller._on_iserver_event("LINKIO_DIAGNOSTIC", text, {
                "stream": "stderr", "byte_count": len(text.encode()),
                "classification": "TRANSPORT_DIAGNOSTIC"})
            while True:
                event = controller.events.get_nowait()
                if event["kind"] == "LINKIO_DIAGNOSTIC":
                    break
            self.assertEqual(controller.snapshot()["iserver_stderr_bytes"], len(text.encode()))
            self.assertEqual(event["kind"], "LINKIO_DIAGNOSTIC")
            self.assertEqual(event["operation"], "LINKIO/ReadLink")
            self.assertEqual(event["details"]["classification"], "TRANSPORT_DIAGNOSTIC")
            controller.close()

    def test_gui_keeps_linkio_diagnostic_out_of_occam_console(self):
        import gui.app as app_module
        old_support = app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as tmp:
            app_module.APP_SUPPORT = Path(tmp) / "support"
            root = tk.Tk()
            root.withdraw()
            app = app_module.ControlCenter(root)
            payload = "[LINKIO_DIAG] seq=3 event=summary reason=COUNT_COMPLETE\n"
            app._display_event({"seq": 1, "timestamp": "t", "origin": "ISERVER",
                "operation": "LINKIO/ReadLink", "kind": "LINKIO_DIAGNOSTIC",
                "details": {"text": payload, "stream": "stderr", "byte_count": len(payload),
                            "classification": "TRANSPORT_DIAGNOSTIC"}})
            self.assertEqual(app.console.listbox.size(), 0)
            self.assertEqual(app.event_tree.item(app.event_tree.get_children()[-1], "values")[3],
                             "LINKIO_DIAGNOSTIC")
            app._display_event({"seq": 2, "timestamp": "t2", "origin": "ISERVER",
                "operation": "process", "kind": "ISERVER_STDERR",
                "details": {"text": "iserver warning\n", "stream": "stderr", "byte_count": 15}})
            self.assertEqual(app.console.listbox.get("end"), "[stderr] iserver warning")
            app.close()
            app_module.APP_SUPPORT = old_support


if __name__ == "__main__":
    unittest.main()
