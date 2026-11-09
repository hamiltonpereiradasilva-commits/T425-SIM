import json
import os
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from services import controller as controller_module
from services.controller import MonitorController, select_iserver_executable

ROOT = Path(__file__).resolve().parents[1]
DIAG = (ROOT / "build/bin/iserver-linkusb-sp-reply-diag").resolve()
PROD = (ROOT / "build/bin/iserver-linkusb").resolve()


class DiagnosticRunnerSelectionTests(unittest.TestCase):
    def test_environment_selects_only_pinned_diagnostic_executable(self):
        selected, digest, mode = select_iserver_executable(
            ROOT, {"T425_ISERVER_EXECUTABLE": str(DIAG)})
        self.assertEqual(selected, DIAG)
        self.assertEqual(digest, controller_module.DIAGNOSTIC_EXECUTABLE_SHA256)
        self.assertEqual(mode, "ENV_DIAGNOSTIC")
        self.assertTrue(os.access(str(selected), os.X_OK))

    def test_default_selection_is_preserved_without_environment(self):
        selected, digest, mode = select_iserver_executable(ROOT, {})
        self.assertEqual(selected, PROD)
        self.assertTrue(digest)
        self.assertEqual(mode, "DEFAULT")

    def test_environment_rejects_productive_or_arbitrary_executable(self):
        with self.assertRaisesRegex(RuntimeError, "deve apontar para o executável diagnóstico"):
            select_iserver_executable(ROOT, {"T425_ISERVER_EXECUTABLE": str(PROD)})
        with self.assertRaises((FileNotFoundError, RuntimeError)):
            select_iserver_executable(ROOT, {"T425_ISERVER_EXECUTABLE": str(ROOT / "missing")})

    def test_digest_mismatch_is_rejected(self):
        with mock.patch.object(controller_module, "DIAGNOSTIC_EXECUTABLE_SHA256", "0" * 64):
            with self.assertRaisesRegex(RuntimeError, "SHA-256.*diverge"):
                select_iserver_executable(ROOT, {"T425_ISERVER_EXECUTABLE": str(DIAG)})

    def test_controller_logs_selection_without_usb_or_iserver_start(self):
        with tempfile.TemporaryDirectory() as tmp:
            log = Path(tmp) / "events.jsonl"
            with mock.patch.dict(os.environ, {"T425_ISERVER_EXECUTABLE": str(DIAG)}):
                controller = MonitorController(event_path=log)
            try:
                self.assertEqual(controller.iserver.executable, str(DIAG))
                self.assertIsNone(controller.iserver.process)
                self.assertFalse(controller._worker.is_alive())
                self.assertIsNone(controller._usb)
                events = [json.loads(line) for line in log.read_text().splitlines()]
                chosen = [e for e in events if e["kind"] == "ISERVER_EXECUTABLE_SELECTED"]
                self.assertEqual(len(chosen), 1)
                self.assertEqual(chosen[0]["details"]["path"], str(DIAG))
                self.assertEqual(chosen[0]["details"]["selection"], "ENV_DIAGNOSTIC")
            finally:
                controller.close()


if __name__ == "__main__":
    unittest.main()
