#!/usr/bin/env bash
set -euo pipefail

FILE="source/firmware_pico/t425_uart8_8n1.pio"
BACKUP="${FILE}.before_rx_fix_$(date +%Y%m%d_%H%M%S)"

echo "========================================"
echo " T425 PIO RX FIX"
echo "========================================"

if [ ! -f "$FILE" ]; then
    echo "ERRO: arquivo nao encontrado: $FILE"
    exit 1
fi

cp "$FILE" "$BACKUP"

echo
echo "Backup criado:"
echo "  $BACKUP"

python3 - <<'PY'
from pathlib import Path
import re

path = Path("source/firmware_pico/t425_uart8_8n1.pio")
text = path.read_text()

old = r'''\.program t425_uart_rx.*?\.wrap\s*$'''

new = '''.program t425_uart_rx

.wrap_target

wait_start:
    wait 0 pin 0

    ; 1.5 bit-times from start detection to center of data bit 0
    ; PIO clock = baud * 8
    nop [7]
    nop [3]

    set x, 7

rxloop:
    in pins, 1 [6]
    jmp x-- rxloop

    push block

.wrap
'''

updated, count = re.subn(
    old,
    new,
    text,
    flags=re.DOTALL | re.MULTILINE
)

if count != 1:
    raise SystemExit(
        f"ERRO: esperava substituir exatamente 1 bloco RX, encontrei {count}"
    )

path.write_text(updated)

print("RX atualizado com sucesso.")
PY

echo
echo "----------------------------------------"
echo "NOVO PROGRAMA PIO:"
echo "----------------------------------------"

cat "$FILE"

echo
echo "========================================"
echo "FIX CONCLUIDO"
echo "========================================"
