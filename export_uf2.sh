#!/usr/bin/env bash

set -euo pipefail

ROOT="/work/t425-canonical"
UF2="$ROOT/build-firmware-pico/t425_pico_v15.uf2"
OUTDIR="$ROOT/export"
OUT="$OUTDIR/t425_pico_v15.uf2"

echo "========================================"
echo " T425-SIM UF2 EXPORT"
echo "========================================"
echo

if [ ! -f "$UF2" ]; then
    echo "ERRO: firmware UF2 nao encontrado:"
    echo "  $UF2"
    echo
    echo "Execute primeiro o build."
    exit 1
fi

mkdir -p "$OUTDIR"

echo "Firmware encontrado:"
ls -lh "$UF2"

echo
echo "Copiando para:"
echo "  $OUT"

cp -f "$UF2" "$OUT"

echo
echo "========================================"
echo " EXPORT CONCLUIDO"
echo "========================================"
echo

ls -lh "$OUT"

echo
echo "Arquivo pronto:"
echo "  $OUT"
