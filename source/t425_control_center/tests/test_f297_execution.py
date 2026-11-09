import tempfile
import time
import unittest
from pathlib import Path

from services.btl import inspect_btl
from services.controller import MonitorController


class FakeUsbSession:
    def __init__(self):
        self.closed = False

    def close(self):
        self.closed = True

    def b4(self, link):
        return {
            "link": link, "started": True, "tx_ready": True, "tx_waiting_ack": False,
            "data_pending": False, "physical_ack_pending": False,
            "host_tx_queue_depth": 0, "host_rx_queue_depth": 0, "malformed_words": 0,
        }


class FakeIServer:
    def __init__(self):
        self.process = None
        self.started = []

    def start(self, path):
        self.started.append(path)
        self.process = object()
        return 4242

    def stop(self):
        self.process = None


class F297ExecutionGateTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.controller = MonitorController(event_path=Path(self.tmp.name) / "events.jsonl")
        self.btl_path = Path(__file__).resolve().parents[1] / "examples-hello-t425-test.btl"
        self.btl = inspect_btl(self.btl_path)

    def tearDown(self):
        self.controller.close()
        self.tmp.cleanup()

    def set_ready_status(self, boot_state=2, controller=None):
        controller = controller or self.controller
        now = time.monotonic()
        controller._usb = FakeUsbSession()
        controller.state["linkusb"] = "CONECTADO — diagnóstico B4 somente leitura"
        controller.state["linkusb_observed_monotonic"] = now
        controller.state["linkusb_b4"] = {0: {
            "link": 0, "started": True, "tx_ready": True, "tx_waiting_ack": False,
            "data_pending": False, "physical_ack_pending": False,
            "host_tx_queue_depth": 0, "host_rx_queue_depth": 0, "malformed_words": 0,
        }}
        controller.state["t425_status"] = {
            "protocol": 1, "startup_stage": "RUNNING", "core1_state": 2, "boot_state": boot_state,
            "failure_reason": None, "failure_code": None,
            "links": [{"endpoint": 0, "available": True}],
        }
        controller.state["t425_observed_monotonic"] = now

    def test_preflight_requires_valid_btl_and_both_fresh_diagnostics(self):
        self.assertFalse(self.controller.execution_preflight(self.btl)["ready"])
        self.set_ready_status()
        self.assertTrue(self.controller.execution_preflight(self.btl)["ready"])

    def test_preflight_rejects_f294_load_state(self):
        self.set_ready_status(boot_state=7)
        gate = self.controller.execution_preflight(self.btl)
        self.assertFalse(gate["ready"])
        self.assertTrue(any("WAIT_CONTROL" in reason for reason in gate["reasons"]))

    def test_preflight_rejects_stale_or_pending_link_state(self):
        self.set_ready_status()
        self.controller.state["linkusb_observed_monotonic"] -= 61
        self.assertFalse(self.controller.execution_preflight(self.btl)["ready"])
        self.set_ready_status()
        self.controller.state["linkusb_b4"][0]["tx_waiting_ack"] = True
        self.assertFalse(self.controller.execution_preflight(self.btl)["ready"])

    def test_start_rechecks_btl_and_gate_before_releasing_usb(self):
        self.set_ready_status(boot_state=7)
        fake_server = FakeIServer()
        self.controller.iserver = fake_server
        session = self.controller._usb
        with self.assertRaisesRegex(RuntimeError, "WAIT_CONTROL"):
            self.controller._start_iserver({
                "path": str(self.btl_path), "sha256": self.btl.sha256, "size": self.btl.size,
            })
        self.assertFalse(session.closed)
        self.assertEqual(fake_server.started, [])

    def test_start_rehashes_then_releases_diagnostic_owner_before_spawn(self):
        self.set_ready_status()
        fake_server = FakeIServer()
        self.controller.iserver = fake_server
        session = self.controller._usb
        result = self.controller._start_iserver({
            "path": str(self.btl_path), "sha256": self.btl.sha256, "size": self.btl.size,
        })
        self.assertTrue(session.closed)
        self.assertEqual(fake_server.started, [str(self.btl_path)])
        self.assertEqual(result["phase"], "PROCESS_STARTED_NOT_BOOT_CONFIRMED")

    def test_changed_btl_is_rejected(self):
        payload = {"path": str(self.btl_path), "sha256": "0" * 64, "size": self.btl.size}
        with self.assertRaisesRegex(RuntimeError, "mudou"):
            self.controller._start_iserver(payload)

    def test_gui_arms_load_only_after_fresh_gate_and_keeps_other_actions_disabled(self):
        import tkinter as tk
        import gui.app as app_module
        old_support = app_module.APP_SUPPORT
        with tempfile.TemporaryDirectory() as support:
            app_module.APP_SUPPORT = Path(support)
            root = tk.Tk(); root.withdraw()
            app = app_module.ControlCenter(root)
            app.btl = self.btl
            self.set_ready_status(controller=app.controller)
            app._refresh_execution_gate()
            self.assertEqual(str(app.arm_check["state"]), "normal")
            self.assertEqual(str(app.load_button["state"]), "disabled")
            app.execution_armed.set(True)
            app._refresh_execution_gate()
            self.assertEqual(str(app.load_button["state"]), "normal")
            self.assertEqual(str(app.run_button["state"]), "disabled")
            self.assertEqual(str(app.stop_button["state"]), "disabled")
            self.assertEqual(str(app.reset_button["state"]), "disabled")
            app.controller.state["t425_status"]["boot_state"] = 7
            app._refresh_execution_gate()
            self.assertEqual(str(app.load_button["state"]), "disabled")
            self.assertFalse(app.execution_armed.get())
            app.close()
            app_module.APP_SUPPORT = old_support


if __name__ == "__main__":
    unittest.main()
