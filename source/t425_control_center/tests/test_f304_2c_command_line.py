import os
import unittest
from pathlib import Path
from unittest import mock

from iserver.iserver_runner import IServerRunner, prepare_inmos_argv

ROOT = Path(__file__).resolve().parents[1]
DIAG = ROOT / "build/bin/iserver-linkusb-sp-reply-diag"
BTL = ROOT / "examples-hello-t425-test.btl"


class CommandLineLimitTests(unittest.TestCase):
    def test_long_actual_paths_shorten_only_argv0_and_keep_exec_and_cwd(self):
        before_cwd = os.getcwd()
        launch = prepare_inmos_argv(DIAG, BTL)
        original_argv = [str(DIAG), "-sc", str(BTL.resolve()), "-ss", "-sl", "linkusb0"]
        shortened_argv = [DIAG.name, "-sc", str(BTL.resolve()), "-ss", "-sl", "linkusb0"]
        self.assertEqual(launch["original_length"], len(" ".join(original_argv)))
        self.assertEqual(launch["final_length"], len(" ".join(shortened_argv)))
        self.assertTrue(launch["shortened"])
        self.assertEqual(launch["argv"][0], DIAG.name)
        self.assertEqual(launch["argv"][2], str(BTL.resolve()))
        self.assertEqual(launch["executable"], str(DIAG.resolve()))
        self.assertEqual(launch["cwd"], before_cwd)
        self.assertLessEqual(launch["final_length"], 255)

    def test_short_command_line_keeps_original_argv0(self):
        launch = prepare_inmos_argv("/tmp/iserver", "/tmp/test.btl")
        self.assertFalse(launch["shortened"])
        self.assertEqual(launch["argv"][0], "/tmp/iserver")
        self.assertIsNone(launch["executable"])

    def test_still_oversize_after_argv0_shortening_is_rejected(self):
        long_btl = "/tmp/" + ("x" * 280) + ".btl"
        with self.assertRaisesRegex(ValueError, "acima do limite INMOS"):
            prepare_inmos_argv(DIAG, long_btl)

    def test_runner_uses_short_argv0_and_exec_override_without_real_process(self):
        events = []
        runner = IServerRunner(str(DIAG), lambda *event: events.append(event), enabled=True)
        class FakeProcess:
            pid = 5151
            def __init__(self):
                self.stdout = self._eof_pipe()
                self.stderr = self._eof_pipe()
            @staticmethod
            def _eof_pipe():
                rfd, wfd = os.pipe()
                os.close(wfd)
                return os.fdopen(rfd, "rb", buffering=0)
            def wait(self):
                return 0
        fake = FakeProcess()
        with mock.patch("iserver.iserver_runner.subprocess.Popen", return_value=fake) as popen:
            pid = runner.start(str(BTL))
            self.assertEqual(pid, 5151)
            args, kwargs = popen.call_args
            self.assertEqual(args[0][0], DIAG.name)
            self.assertEqual(kwargs["executable"], str(DIAG.resolve()))
            self.assertNotIn("cwd", kwargs)
        for thread in runner._threads:
            thread.join(timeout=2)
        shortened = [e for e in events if e[0] == "ISERVER_COMMAND_LINE_SHORTENED"]
        self.assertEqual(len(shortened), 1)
        original_argv = [str(DIAG), "-sc", str(BTL.resolve()), "-ss", "-sl", "linkusb0"]
        shortened_argv = [DIAG.name, "-sc", str(BTL.resolve()), "-ss", "-sl", "linkusb0"]
        self.assertEqual(shortened[0][2]["original_length"], len(" ".join(original_argv)))
        self.assertEqual(shortened[0][2]["final_length"], len(" ".join(shortened_argv)))
        self.assertIsNone(runner.process)


if __name__ == "__main__":
    unittest.main()
