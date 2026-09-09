# T425-SIM v2.8 — Pinout físico definitivo

## Topologia

```text
P00 <====> P01 <====> P02 <====> P03
```

Cada enlace é full-duplex, com TX/RX separados e GND comum.

## GPIO

### P00
- GP0: LINK0_TX -> P01
- GP1: LINK0_RX <- P01
- GP2: LINK1_TX -> P01 (reservado para futura expansão)
- GP3: LINK1_RX <- P01 (reservado)

### P01
- GP0: LINK0_TX -> P00
- GP1: LINK0_RX <- P00
- GP2: LINK1_TX -> P02
- GP3: LINK1_RX <- P02

### P02
- GP0: LINK1_TX -> P01
- GP1: LINK1_RX <- P01
- GP2: LINK2_TX -> P03
- GP3: LINK2_RX <- P03

### P03
- GP0: LINK2_TX -> P02
- GP1: LINK2_RX <- P02

## Primeiro teste físico

Somente:
P00 GP0 -> P01 GP1
P01 GP0 -> P00 GP1
GND -> GND

Lógica de 3,3 V. Não aplicar 5 V aos GPIO.
