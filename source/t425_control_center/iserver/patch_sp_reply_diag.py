#!/usr/bin/env python3
"""Apply diagnostic-only probes to disposable build copies; fail closed on drift."""
from pathlib import Path
import sys

root = Path(sys.argv[1])
serve = root / "iserver/src/serve.c"
filec = root / "iserver/src/filec.c"
lops = root / "linkops/src/lops.c"
pack = root / "iserver/src/pack.c"


def replace_once(path, old, new, label):
    text = path.read_text()
    if text.count(old) != 1:
        raise SystemExit(f"expected exactly one {label} anchor in {path}; found {text.count(old)}")
    path.write_text(text.replace(old, new, 1))

# The diagnostic helper and pack globals are included only in disposable copies.
for path in (serve, filec, lops, pack):
    text = path.read_text()
    marker = '#include "sp_reply_diag.h"\n'
    if marker in text:
        raise SystemExit(f"diagnostic include already present in {path}")
    includes = text.find('\n\n')
    if includes < 0:
        raise SystemExit(f"cannot locate include block in {path}")
    text = text[:includes+2] + marker + text[includes+2:]
    path.write_text(text)

# Serve: record a completed SP_PUTS request and the reply buffer immediately before send.
replace_once(
    serve,
    '      ops_res = OPS_GetRequest(ConnId, Tbuf, ONESECOND/6, HeartBeat);\n',
    '      ops_res = OPS_GetRequest(ConnId, Tbuf, ONESECOND/6, HeartBeat);\n'
    '      { unsigned long long spdiag_txn = sp_reply_diag_begin(ops_res == STATUS_NOERROR && Tbuf[2] == SP_PUTS);\n'
    '        if (sp_reply_diag_is_puts())\n'
    '          sp_reply_diag_record("A_AFTER_OPS_GET_REQUEST", spdiag_txn, (int)ops_res,\n'
    '              Tbuf, OutBuf, InBuf, OutCount, -1, Tbuf);\n'
    '      }\n',
    'OPS_GetRequest call')
replace_once(
    serve,
    '      ops_res = OPS_SendReply(ConnId, Tbuf);\n',
    '      if (sp_reply_diag_is_puts())\n'
    '         sp_reply_diag_record("C_BEFORE_OPS_SEND_REPLY", sp_reply_diag_current(), -1,\n'
    '             Tbuf, OutBuf, InBuf, OutCount, (int)Tbuf[0] + 256*(int)Tbuf[1], Tbuf);\n'
    '      ops_res = OPS_SendReply(ConnId, Tbuf);\n',
    'OPS_SendReply call')
# Required globals are declared by pack.h; add it to the disposable Serve translation unit.
text = serve.read_text()
needle = '#include "linkops.h"\n'
if text.count(needle) != 1:
    raise SystemExit("Serve pack.h anchor not unique")
serve.write_text(text.replace(needle, needle + '#include "pack.h"\n', 1))

# SpPuts: log entry before pointer setup and completion after put_count writes length header.
text = filec.read_text()
start = text.find('void            SpPuts()')
end = text.find('\n/*\n * SpFlush', start)
if start < 0 or end < 0:
    raise SystemExit('cannot isolate SpPuts body')
part = text[start:end]
replace_once(filec, part,
    part.replace('   dbgmsg("SP.PUTS");\n',
        '   sp_reply_diag_record("B_SP_PUTS_ENTRY", sp_reply_diag_current(), -1,\n'
        '       Tbuf, OutBuf, InBuf, OutCount, -1, Tbuf);\n'
        '   dbgmsg("SP.PUTS");\n')
        .replace('   put_count(OutCount);\n',
        '   sp_reply_diag_put_count("E_SP_PUTS_BEFORE_PUT_COUNT", sp_reply_diag_current(),\n'
        '       (unsigned int)OutCount, &OutCount, Tbuf, OutBuf, OutCount);\n'
        '   put_count(OutCount);\n'
        '   sp_reply_diag_put_count("E_SP_PUTS_AFTER_PUT_COUNT", sp_reply_diag_current(),\n'
        '       (unsigned int)OutCount, &OutCount, Tbuf, OutBuf, OutCount);\n'
        '   sp_reply_diag_record("B_SP_PUTS_EXIT", sp_reply_diag_current(), -1,\n'
        '       Tbuf, OutBuf, InBuf, OutCount, (int)Tbuf[0] + 256*(int)Tbuf[1], Tbuf);\n'),
    'SpPuts body')

# put_count probes are restricted to the diagnostic build copy and only active
# while the correlated SP_PUTS transaction is being processed.
text = pack.read_text()
start = text.find('void put_count(len)')
end = text.find('\n/* Pack bottom 16-bits', start)
if start < 0 or end < 0:
    raise SystemExit('cannot isolate put_count body')
part = text[start:end]
entry_anchor = '   register unsigned char *cp;\n'
if part.count(entry_anchor) != 1:
    raise SystemExit('put_count local declaration anchor is not unique')
part = part.replace(entry_anchor, entry_anchor +
    '   if (sp_reply_diag_is_puts())\n'
    '      sp_reply_diag_put_count("E_PUT_COUNT_ENTRY", sp_reply_diag_current(),\n'
    '          len, &len, Tbuf, OutBuf, OutCount);\n', 1)
write_anchor = '#endif /* BIG_ENDIAN */\n}'
if part.count(write_anchor) != 1:
    raise SystemExit('put_count post-write anchor is not unique')
part = part.replace(write_anchor,
    '#endif /* BIG_ENDIAN */\n'
    '   if (sp_reply_diag_is_puts())\n'
    '      sp_reply_diag_put_count("E_PUT_COUNT_AFTER_HEADER_WRITE", sp_reply_diag_current(),\n'
    '          len, &len, Tbuf, OutBuf, OutCount);\n'
    '}', 1)
# The Darwin headers define BIG_ENDIAN as a constant even on little-endian
# hosts. The legacy #ifdef therefore selects the high-byte path on macOS.
# Serialize the SP 16-bit length explicitly in protocol little-endian order.
legacy_endian_block = (
    '#ifdef BIG_ENDIAN\n'
    '   cp = (unsigned char *)&len+sizeof(int)-1;\n'
    '   \n'
    '   Tbuf[0] = *cp--;\n'
    '   Tbuf[1] = *cp;\n'
    '#else\n'
    '   cp = (unsigned char *)&len;\n'
    '   \n'
    '   Tbuf[0] = *cp++;\n'
    '   Tbuf[1] = *cp;\n'
    '#endif /* BIG_ENDIAN */\n')
explicit_little_endian = (
    '   Tbuf[0] = (unsigned char)(len & 0xFFU);\n'
    '   Tbuf[1] = (unsigned char)((len >> 8) & 0xFFU);\n')
if part.count(legacy_endian_block) != 1:
    raise SystemExit(f'expected one put_count endian block; found {part.count(legacy_endian_block)}')
part = part.replace(legacy_endian_block, explicit_little_endian, 1)
text = text[:start] + part + text[end:]

# SP integer fields are little-endian on the wire, independently of host
# byte order and sizeof(long). Patch only the disposable generated build copy;
# leave the original INMOS source tree untouched.
get32_start = text.find('unsigned long get_32()')
get32_end = text.find('\nunsigned int get_slice', get32_start)
if get32_start < 0 or get32_end < 0:
    raise SystemExit('cannot isolate get_32 body')
get32_original = text[get32_start:get32_end]
get32_expected = '''unsigned long get_32()
{
   long                   res=0L;
   register unsigned char *cp;
   
#ifdef BIG_ENDIAN
   cp = (unsigned char *)&res+sizeof(long)-1;
   
   *cp-- = *InBuf++;
   *cp-- = *InBuf++;
   *cp-- = *InBuf++;
   *cp-- = *InBuf++;
#else
   cp = (unsigned char *)&res;
   
   *cp++ = *InBuf++;
   *cp++ = *InBuf++;
   *cp++ = *InBuf++;
   *cp++ = *InBuf++;
#endif /* BIG_ENDIAN */

   return res;
}
'''
if get32_original != get32_expected:
    raise SystemExit('get_32 body differs from the reviewed INMOS anchor')
get32_explicit = '''unsigned long get_32()
{
   unsigned long res;

   res = (unsigned long)InBuf[0]
       | ((unsigned long)InBuf[1] << 8)
       | ((unsigned long)InBuf[2] << 16)
       | ((unsigned long)InBuf[3] << 24);
   InBuf += 4;
   return res;
}'''
text = text[:get32_start] + get32_explicit + text[get32_end:]
pack.write_text(text)

# LOPS_SendReply: observe the actual argument at entry, before header validation.
anchor = '  DebugMessage (fprintf (stderr, "[LOPS_SendReply\\n") );\n'
replace_once(lops, anchor,
    '  if (sp_reply_diag_is_puts() && buffer != NULL)\n'
    '    sp_reply_diag_record("D_LOPS_SEND_REPLY_ENTRY", sp_reply_diag_current(), -1,\n'
    '        NULL, NULL, NULL, -1, -1, buffer);\n'
    + anchor,
    'LOPS_SendReply entry')

# Context for the original LOPS_GetRequest ReadLink calls. The probe is gated
# by T425_LINKIO_READ_TRACE and does not inspect or modify the returned bytes
# unless the separate payload-capture opt-in is also enabled.
replace_once(lops,
    '    bytes_read = ReadLink ((int) con_id, (char *) buffer, 1, wait_time);\n',
    '    sp_reply_diag_link_read("SP_HEADER_FIRST_BYTE", "BEGIN", 0u, 1u, -1, buffer);\n'
    '    bytes_read = ReadLink ((int) con_id, (char *) buffer, 1, wait_time);\n'
    '    sp_reply_diag_link_read("SP_HEADER_FIRST_BYTE", "END", 0u, 1u, bytes_read, buffer);\n',
    'first SP header ReadLink')
replace_once(lops,
    '        nread = ReadLink ((int) con_id, (char *) &buffer[1], (unsigned int) (8 - 1), IOSHORT_TIMEOUT);\n',
    '        sp_reply_diag_link_read("SP_HEADER_REMAINDER", "BEGIN", 1u, 7u, -1, &buffer[1]);\n'
    '        nread = ReadLink ((int) con_id, (char *) &buffer[1], (unsigned int) (8 - 1), IOSHORT_TIMEOUT);\n'
    '        sp_reply_diag_link_read("SP_HEADER_REMAINDER", "END", 1u, 7u, nread, &buffer[1]);\n',
    'remaining SP header ReadLink')
replace_once(lops,
    '    bytes_read = ReadLink ((int) con_id, (char *) &buffer[8], (unsigned int) bytes_to_read, IOSHORT_TIMEOUT);\n',
    '    sp_reply_diag_link_read("SP_PACKET_BODY", "BEGIN", 8u, (unsigned int)bytes_to_read, -1, &buffer[8]);\n'
    '    bytes_read = ReadLink ((int) con_id, (char *) &buffer[8], (unsigned int) bytes_to_read, IOSHORT_TIMEOUT);\n'
    '    sp_reply_diag_link_read("SP_PACKET_BODY", "END", 8u, (unsigned int)bytes_to_read, bytes_read, &buffer[8]);\n',
    'SP packet body ReadLink')
