# T425-SIM RP2040 — v1.2 PIO

A etapa define o transporte UART-like em PIO, 8 bits, full-duplex e dois links por nó. O alvo inicial é 2 Mbaud.

```text
RP2040
 PIO/SM TX0 ─────────► RX0
 PIO/SM RX0 ◄───────── TX0
 PIO/SM TX1 ─────────► RX1
 PIO/SM RX1 ◄───────── TX1
```

O `t425_uart8.pio` é a especificação inicial das state machines. A adaptação Pico SDK está isolada em `firmware_pico/`.

**Não declarar PASS físico ainda:** clock divider, amostragem RX, pinagem e integridade elétrica precisam ser validados na bancada.
