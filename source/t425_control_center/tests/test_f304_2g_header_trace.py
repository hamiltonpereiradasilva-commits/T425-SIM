import hashlib
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build/sp-reply-diag-src"


class HeaderTraceInstrumentationTests(unittest.TestCase):
    def test_generated_lops_traces_each_readlink_phase(self):
        src = (BUILD / "linkops/src/lops.c").read_text()
        for marker in (
            '"SP_HEADER_FIRST_BYTE", "BEGIN"',
            '"SP_HEADER_FIRST_BYTE", "END"',
            '"SP_HEADER_REMAINDER", "BEGIN"',
            '"SP_HEADER_REMAINDER", "END"',
            '"SP_PACKET_BODY", "BEGIN"',
            '"SP_PACKET_BODY", "END"',
        ):
            self.assertIn(marker, src)
        self.assertLess(src.index('"SP_HEADER_REMAINDER", "BEGIN"'),
                        src.index('nread = ReadLink'))
        self.assertLess(src.index('nread = ReadLink'),
                        src.index('"SP_HEADER_REMAINDER", "END"'))

    def test_trace_is_opt_in_and_payload_is_bounded(self):
        helper = (ROOT / "iserver/sp_reply_diag.c").read_text()
        self.assertIn('strcmp(trace, "1") != 0', helper)
        self.assertIn('strcmp(capture, "I_UNDERSTAND") == 0', helper)
        self.assertIn('if (bytes > 8u) bytes = 8u;', helper)
        self.assertIn('clock_gettime(CLOCK_MONOTONIC', helper)
        backend = (ROOT / "iserver/linkusb_linkio.c").read_text()
        self.assertIn("session_open=%d device_handle=%d lock_held=%d", backend)

    def test_original_lops_is_unchanged_and_diagnostic_binary_is_pinned(self):
        original = ROOT / "iserver/original/linkops/src/lops.c"
        self.assertNotIn("sp_reply_diag_link_read", original.read_text())
        binary = ROOT / "build/bin/iserver-linkusb-sp-reply-diag"
        digest = hashlib.sha256(binary.read_bytes()).hexdigest()
        from services.controller import DIAGNOSTIC_EXECUTABLE_SHA256
        self.assertEqual(digest, DIAGNOSTIC_EXECUTABLE_SHA256)


if __name__ == "__main__":
    unittest.main()
