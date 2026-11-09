import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GENERATED = ROOT / "build/sp-reply-diag-src/iserver/src/pack.c"
ORIGINAL = ROOT / "iserver/original/iserver/src/pack.c"


class Get32EndianFixTests(unittest.TestCase):
    def test_generated_get32_is_explicit_little_endian(self):
        source = GENERATED.read_text()
        start = source.index("unsigned long get_32()")
        end = source.index("unsigned int get_slice", start)
        body = source[start:end]
        self.assertIn("(unsigned long)InBuf[0]", body)
        self.assertIn("((unsigned long)InBuf[3] << 24)", body)
        self.assertIn("InBuf += 4;", body)
        self.assertNotIn("BIG_ENDIAN", body)

    def test_put_count_fix_remains_applied_and_original_is_unmodified(self):
        generated = GENERATED.read_text()
        put_start = generated.index("void put_count(len)")
        put_end = generated.index("/* Pack bottom 16-bits", put_start)
        self.assertIn("Tbuf[0] = (unsigned char)(len & 0xFFU);", generated[put_start:put_end])
        original = ORIGINAL.read_text()
        get_start = original.index("unsigned long get_32()")
        get_end = original.index("unsigned int get_slice", get_start)
        self.assertIn("#ifdef BIG_ENDIAN", original[get_start:get_end])
        self.assertNotIn("InBuf += 4;", original[get_start:get_end])


if __name__ == "__main__":
    unittest.main()
