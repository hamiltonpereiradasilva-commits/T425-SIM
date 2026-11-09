#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORIG="$ROOT/iserver/original"
WORK="$ROOT/build/iserver-src"
OUT="$ROOT/build/bin"
mkdir -p "$WORK" "$OUT"
rm -rf "$WORK/iserver" "$WORK/linkops" "$WORK/linkios"
cp -R "$ORIG/iserver" "$WORK/"
cp -R "$ORIG/linkops" "$WORK/"
cp -R "$ORIG/linkios" "$WORK/"
python3 "$ROOT/iserver/patch_linkops.py" "$WORK/linkops/src/linkops.c"
python3 "$ROOT/iserver/patch_macos_portability.py" "$WORK/iserver/src"
USB_CFLAGS=$(pkg-config --cflags libusb-1.0)
USB_LIBS=$(pkg-config --libs libusb-1.0)
CC=${CC:-clang}
CFLAGS="-O2 -std=gnu89 -fcommon -DF296_MACOS -Wno-error=implicit-function-declaration -Wno-deprecated-non-prototype -Wno-implicit-int -Wno-int-conversion -Wno-incompatible-function-pointer-types -include $WORK/iserver/src/misc.h -include $WORK/iserver/src/pack.h -include $ROOT/iserver/iserver_compat.h -include $ROOT/iserver/linkusb_linkio.h -include stdlib.h -include string.h -include unistd.h -I$WORK/iserver/src -I$WORK/linkops/src -I$WORK/linkios/src -I$ROOT/iserver $USB_CFLAGS"
SOURCES="$WORK/iserver/src/iserver.c $WORK/iserver/src/hostc.c $WORK/iserver/src/filec.c $WORK/iserver/src/serverc.c $WORK/iserver/src/sh.c $WORK/iserver/src/serve.c $WORK/iserver/src/boot.c $WORK/iserver/src/pack.c $WORK/iserver/src/info.c $WORK/iserver/src/ttyio.c $WORK/iserver/src/files.c $WORK/iserver/src/parsecl.c $WORK/iserver/src/record.c $WORK/iserver/src/misc.c $WORK/linkops/src/linkops.c $WORK/linkops/src/conlib.c $WORK/linkops/src/debug.c $WORK/linkops/src/lmethod.c $WORK/linkops/src/lops.c $ROOT/iserver/linkusb_linkio.c"
# shellcheck disable=SC2086
$CC $CFLAGS $SOURCES $USB_LIBS -o "$OUT/iserver-linkusb"
file "$OUT/iserver-linkusb"
