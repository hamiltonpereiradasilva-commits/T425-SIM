#!/bin/sh
set -eu
source=source/t425_phy/rp2040/t425_link_rx.pio
first=/tmp/t425_f_phy_68_rx_1_v3.pio.h
second=/tmp/t425_f_phy_68_rx_2_v3.pio.h
pioasm -o c-sdk "$source" "$first"
pioasm -o c-sdk "$source" "$second"
cmp -s "$first" "$second"
python3 - <<'PY'
from pathlib import Path
source = Path("source/t425_phy/rp2040/t425_link_rx.pio").read_text()
lines = source.splitlines()
samples = [i for i, line in enumerate(lines) if line.strip() == "in pins, 1"]
irqs = [i for i, line in enumerate(lines) if line.strip() == "irq nowait 0 rel"]
assert len(samples) >= 2 and len(irqs) == 1
assert irqs[0] > samples[1]
assert "jmp pin, rx_data_event" in source
assert "jmp rx_data_continue" in source
assert "push block" in source
adapter = Path("source/t425_phy/rp2040/core_adapter/t425_core_phy_rp2040.c").read_text()
assert "pio_interrupt_get(pio, link->sm_rx)" in adapter
assert "t425_link_send_ack(link)" in adapter
assert "t425_link_note_ack_received(link)" in adapter
assert "physical_ack_pending[i] = false" in adapter
assert "physical_ack_count[i] = 0u" in adapter
assert "T425_RX_ACK_WORD    0x80000000u" in adapter
assert "T425_RX_TYPE_BIT    30u" in adapter
PY
grep -F "return 15000000.0f" source/t425_phy/rp2040/t425_link.c >/dev/null
grep -F "return 30000000.0f" source/t425_phy/rp2040/t425_link.c >/dev/null
grep -F "return 60000000.0f" source/t425_phy/rp2040/t425_link.c >/dev/null
printf '%s\n' "F_PHY-68 DATA-qualified IRQ source: PASS"
