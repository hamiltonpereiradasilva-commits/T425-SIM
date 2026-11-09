import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GENERATED = ROOT / "build/sp-reply-diag-src/iserver/src/pack.c"
ORIGINAL = ROOT / "iserver/original/iserver/src/pack.c"


class SPLengthSerializationTests(unittest.TestCase):
    def test_generated_put_count_uses_explicit_wire_order(self):
        source = GENERATED.read_text()
        start = source.index("void put_count(len)")
        end = source.index("/* Pack bottom 16-bits", start)
        body = source[start:end]
        self.assertIn("Tbuf[0] = (unsigned char)(len & 0xFFU);", body)
        self.assertIn("Tbuf[1] = (unsigned char)((len >> 8) & 0xFFU);", body)
        self.assertNotIn("#ifdef BIG_ENDIAN", body)

    def test_inmos_original_remains_unchanged(self):
        source = ORIGINAL.read_text()
        self.assertIn("#ifdef BIG_ENDIAN", source)
        self.assertIn("Tbuf[0] = *cp++;", source)
        self.assertIn("Tbuf[1] = *cp;", source)


if __name__ == "__main__":
    unittest.main()
