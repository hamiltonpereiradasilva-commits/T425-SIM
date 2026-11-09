#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PYTHON=${PYTHON:-$ROOT/.venv314/bin/python}
export PYTHONPATH="$ROOT"
"$PYTHON" -m unittest discover -s "$ROOT/tests" -p 'test_*.py' -v
CC=${CC:-clang}
USB_CFLAGS=$(pkg-config --cflags libusb-1.0)
USB_LIBS=$(pkg-config --libs libusb-1.0)
$CC -std=gnu89 -I"$ROOT/iserver" $USB_CFLAGS "$ROOT/tests/linkio_backend_test.c" "$ROOT/iserver/linkusb_linkio.c" $USB_LIBS -o "$ROOT/build/linkio_backend_test"
"$ROOT/build/linkio_backend_test"
$CC -std=gnu89 -Wall -Wextra -DT425_LINKIO_ENABLE_TEST_HOOK -I"$ROOT/iserver" $USB_CFLAGS "$ROOT/tests/linkio_libusb_injection_test.c" "$ROOT/iserver/linkusb_linkio.c" $USB_LIBS -o "$ROOT/build/linkio_libusb_injection_test"
"$ROOT/build/linkio_libusb_injection_test"
$CC -std=gnu89 -Wno-implicit-function-declaration -Wno-deprecated-non-prototype -include string.h -ffunction-sections -fdata-sections -DT425_LINKIO_ENABLE_TEST_HOOK -I"$ROOT/iserver/original/iserver/linkops/src" -I"$ROOT/iserver/original/iserver/linkios/src" -I"$ROOT/iserver" $USB_CFLAGS "$ROOT/tests/iserver_readlink_integration_test.c" "$ROOT/iserver/original/iserver/linkops/src/lops.c" "$ROOT/iserver/linkusb_linkio.c" $USB_LIBS -Wl,-dead_strip -o "$ROOT/build/iserver_readlink_integration_test"
"$ROOT/build/iserver_readlink_integration_test"
DIAG_WORK="$ROOT/build/sp-reply-diag-src"
$CC -std=gnu89 -fcommon -ffunction-sections -fdata-sections \
  -include "$DIAG_WORK/iserver/src/misc.h" \
  -include "$DIAG_WORK/iserver/src/pack.h" \
  -include "$ROOT/iserver/iserver_compat.h" \
  -include "$ROOT/iserver/linkusb_linkio.h" \
  -include stdlib.h -include string.h -include unistd.h \
  -I"$DIAG_WORK/iserver/src" -I"$DIAG_WORK/linkops/src" \
  -I"$DIAG_WORK/linkios/src" -I"$ROOT/iserver" \
  "$ROOT/tests/put_count_serialization_test.c" \
  "$DIAG_WORK/iserver/src/pack.c" -Wl,-dead_strip \
  -o "$ROOT/build/put_count_serialization_test"
"$ROOT/build/put_count_serialization_test"
$CC -std=gnu89 -fcommon -ffunction-sections -fdata-sections \
  -include "$DIAG_WORK/iserver/src/misc.h" \
  -include "$DIAG_WORK/iserver/src/pack.h" \
  -include "$ROOT/iserver/iserver_compat.h" \
  -include "$ROOT/iserver/linkusb_linkio.h" \
  -include stdlib.h -include string.h -include unistd.h \
  -I"$DIAG_WORK/iserver/src" -I"$DIAG_WORK/linkops/src" \
  -I"$DIAG_WORK/linkios/src" -I"$ROOT/iserver" \
  "$ROOT/tests/get32_wire_test.c" \
  "$DIAG_WORK/iserver/src/pack.c" "$DIAG_WORK/iserver/src/filec.c" \
  "$DIAG_WORK/iserver/src/files.c" \
  -Wl,-dead_strip -o "$ROOT/build/get32_wire_test"
"$ROOT/build/get32_wire_test"
$CC -std=gnu89 -Wall -Wextra -DT425_LINKIO_WRITE_DIAG_TEST -I"$ROOT/iserver" $USB_CFLAGS \
  "$ROOT/tests/linkio_writelink_diagnostic_test.c" "$ROOT/iserver/linkusb_linkio.c" $USB_LIBS \
  -o "$ROOT/build/linkio_writelink_diagnostic_test"
"$ROOT/build/linkio_writelink_diagnostic_test"
