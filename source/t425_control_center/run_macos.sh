#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PYTHON=${PYTHON:-$ROOT/.venv314/bin/python}
if [ ! -x "$PYTHON" ]; then
  echo "Execute ./install_macos.sh com Python 3.14/Tk 9.1 primeiro." >&2
  exit 1
fi
exec "$PYTHON" "$ROOT/main.py"
