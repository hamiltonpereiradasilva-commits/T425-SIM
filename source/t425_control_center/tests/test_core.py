import json
import os
import tempfile
import unittest
import threading
import time
from unittest import mock
from pathlib import Path

from services.btl import inspect_btl, HELLO_SHA256
from linkusb.b4 import decode_b4, B4Error
from services.controller import MonitorController
from iserver.iserver_runner import IServerRunner

class CoreTests(unittest.TestCase):
    def test_hello_btl_hash_and_first_byte(self):
        path = Path.home() / "T425_OUTPUT/F271_PHY_BILATERAL/hello-t425-test.btl"
        if not path.exists():
            self.skipTest("historical Hello World artifact absent")
        info = inspect_btl(path)
        self.assertEqual(info.size, 3267)
        self.assertEqual(info.sha256, HELLO_SHA256)
        self.assertEqual(info.first_byte, 0xF0)
        self.assertTrue(info.expected_hello)

    def test_btl_selection_rejects_empty(self):
        with tempfile.NamedTemporaryFile() as f:
            with self.assertRaises(ValueError):
                inspect_btl(f.name)

    def test_b4_decode(self):
        raw=bytearray(56)
        raw[:4]=b"F271"; raw[4]=1; raw[5]=0; raw[6]=3
        raw[8:12]=(17).to_bytes(4,"little")
        raw[20:24]=(4).to_bytes(4,"little")
        raw[40:42]=(2).to_bytes(2,"little")
        raw[48:52]=(1).to_bytes(4,"little")
        d=decode_b4(raw,0)
        self.assertEqual(d["usb_out_bytes"],17)
        self.assertEqual(d["ack_rx_frames"],4)
        self.assertEqual(d["host_tx_queue_depth"],2)
        self.assertTrue(d["started"] and d["tx_waiting_ack"])
        self.assertFalse(d["physical_pin_capture"])

    def test_b4_rejects_wrong_length_or_link(self):
        with self.assertRaises(B4Error):
            decode_b4(b"")
        raw=bytearray(56); raw[:4]=b"F271"; raw[4]=1; raw[5]=1
        with self.assertRaises(B4Error):
            decode_b4(raw,0)

    def test_controller_starts_without_opening_usb_or_sending(self):
        c=MonitorController(event_path=Path(tempfile.gettempdir())/"f296-test-events.jsonl")
        c.start()
        try:
            snap=c.snapshot()
            self.assertEqual(snap["linkusb"],"DESCONECTADO")
            self.assertEqual(snap["execution"],"DIAGNÓSTICO")
            self.assertIsNone(snap["btl"])
        finally:
            c.close()

    def test_iserver_runner_guard(self):
        r=IServerRunner("/bin/echo",lambda *args:None,enabled=False)
        with self.assertRaises(RuntimeError):
            r.start("/tmp/demo.btl")

    def test_gui_window_opens_and_closes_without_hardware(self):
        import tkinter as tk
        import gui.app as app_module
        old_support=app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as tmp:
            app_module.APP_SUPPORT=Path(tmp)/"support"
            root=tk.Tk(); root.withdraw()
            app=app_module.ControlCenter(root)
            root.update_idletasks()
            self.assertEqual(app.controller.snapshot()["linkusb"], "DESCONECTADO")
            self.assertEqual(str(app.load_button["state"]), "disabled")
            self.assertEqual(str(app.run_button["state"]), "disabled")
            self.assertEqual(str(app.stop_button["state"]), "disabled")
            self.assertEqual(str(app.reset_button["state"]), "disabled")
            self.assertEqual(app.console.listbox.winfo_class(), "Listbox")
            self.assertEqual(app.console.listbox.size(), 0)
            self.assertGreaterEqual(app.log_list.size(), 1)
            app._log("evento de regressão")
            self.assertEqual(app.log_list.get("end"), "evento de regressão")
            app.clear_console()
            self.assertEqual(app.console.listbox.size(), 0)
            app.close()
            app_module.APP_SUPPORT=old_support

    def test_gui_btl_selection_displays_hash_without_loading(self):
        import tkinter as tk
        import gui.app as app_module
        sample=Path(__file__).resolve().parents[1]/"examples-hello-t425-test.btl"
        old_support=app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as tmp:
            app_module.APP_SUPPORT=Path(tmp)/"support"
            root=tk.Tk(); root.withdraw()
            app=app_module.ControlCenter(root)
            with mock.patch.object(app_module.filedialog,"askopenfilename",return_value=str(sample)):
                app.select_btl()
            self.assertTrue(app.btl.expected_hello)
            self.assertEqual(app.btl.first_byte,0xF0)
            self.assertEqual(str(app.load_button["state"]),"disabled")
            self.assertEqual(app.controller.snapshot()["linkusb"],"DESCONECTADO")
            app.close()
            app_module.APP_SUPPORT=old_support

    def test_original_iserver_sp_path_sources_are_present(self):
        root=Path(__file__).resolve().parents[1]/"iserver/original/iserver/src"
        self.assertIn("void     Boot()", (root/"boot.c").read_text(errors="replace"))
        self.assertIn("Serve()", (root/"serve.c").read_text(errors="replace"))
        self.assertIn("SpCommand", (root/"serverc.c").read_text(errors="replace"))
        self.assertIn("OPS_SendReply", (root/"serve.c").read_text(errors="replace"))

    def test_linkusb_diagnostic_reads_only_ep0(self):
        from linkusb.diagnostics import LinkUsbDiagnosticSession
        class FakeDevice:
            def __init__(self): self.calls=[]
            def ctrl_transfer(self, *args, **kwargs):
                self.calls.append(args)
                link=args[3]
                raw=bytearray(56); raw[:4]=b"F271"; raw[4]=1; raw[5]=link
                return raw
        dev=FakeDevice(); session=LinkUsbDiagnosticSession(dev)
        self.assertEqual(session.b4(0)["link"],0)
        self.assertEqual(session.b4(1)["link"],1)
        self.assertEqual([c[1] for c in dev.calls],[0xB4,0xB4])

    def test_t425_cdc_query_uses_only_f271_diagnostic_command(self):
        from t425 import cdc
        written=[]
        class FakeSerial:
            def __init__(self,*a,**kw): self.dtr=False; self.rts=False
            def __enter__(self): return self
            def __exit__(self,*a): pass
            def reset_input_buffer(self): pass
            def write(self,data): written.append(data); return len(data)
            def flush(self): pass
            def readline(self): return b'{"startup_stage":"RUNNING","boot_state":"WAIT_CONTROL"}\n'
        with mock.patch.object(cdc,"find_t425_port",return_value="/dev/fake"), mock.patch.object(cdc.serial,"Serial",FakeSerial):
            result=cdc.query_f271()
        self.assertEqual(result["boot_state"],"WAIT_CONTROL")
        self.assertEqual(written,[b"F271?\n"])

    def test_iserver_supervisor_forwards_stdout_stderr(self):
        events=[]; done=threading.Event()
        def cb(kind,text,metadata=None):
            events.append((kind,text,metadata or {}))
            if len(events)>=2: done.set()
        with tempfile.TemporaryDirectory() as tmp:
            script=Path(tmp)/"fake-iserver"
            script.write_text("#!/bin/sh\nprintf '%s\\n' \"$@\" > '" + str(Path(tmp)/"args.txt") + "'\necho 'server output'\necho 'server diagnostic' >&2\n")
            script.chmod(0o755)
            runner=IServerRunner(str(script),cb,enabled=True)
            btl=Path(tmp)/"fake.btl"
            btl.write_bytes(b"btl")
            runner.start(str(btl))
            proc=runner.process
            self.assertTrue(done.wait(3))
            proc.wait(timeout=3)
            self.assertEqual((Path(tmp)/"args.txt").read_text().splitlines(), ["-sc", str(btl), "-ss", "-sl", "linkusb0"])
            proc.stdout.close(); proc.stderr.close()
            runner.process=None
        self.assertTrue(any(kind == "ISERVER_STDOUT" and text == "server output\n" for kind,text,_ in events))
        self.assertTrue(any(kind == "ISERVER_STDERR" and text == "server diagnostic\n" for kind,text,_ in events))
        self.assertTrue(any(kind == "ISERVER_EXIT" and meta.get("exit_code") == 0 for kind,_,meta in events))

if __name__ == "__main__":
    unittest.main()
