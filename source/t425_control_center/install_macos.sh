#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PYTHON=${PYTHON:-python3.14}
"$PYTHON" -c 'import tkinter; print("Tk", tkinter.TkVersion)'
if ! command -v pkg-config >/dev/null 2>&1 || ! pkg-config --exists libusb-1.0; then
  echo "Dependência do backend C ausente. Instale com: brew install libusb pkg-config" >&2
  exit 1
fi
"$PYTHON" -m venv "$ROOT/.venv314"
"$ROOT/.venv314/bin/python" -m pip install --upgrade pip
"$ROOT/.venv314/bin/python" -m pip install -r "$ROOT/requirements.txt"
echo "Instalação concluída: $ROOT/.venv314"
