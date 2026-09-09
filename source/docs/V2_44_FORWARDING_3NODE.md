# T425-SIM v2.44 — encaminhamento de três nós

A v2.44 implementa a primeira prova de roteamento multi-hop:

```text
P00 ---- P01 ---- P02
```

O objetivo é provar a decisão:

```text
P00: DST=2 -> NEXT_HOP=1
P01: DST=2 -> NEXT_HOP=2
P02: DST=2 -> LOCAL_DELIVER
```

## Cabeçalho de forwarding

```text
VERSION
SRC
DST
FINAL_DST
HOP_COUNT
TTL
PAYLOAD_LEN
```

`TTL` evita loops infinitos.

`HOP_COUNT` permite observar quantos saltos foram realizados.

## Separação

O cabeçalho de forwarding fica acima do Reliable Link:

```text
OCCAM
  |
  v
FORWARDING HEADER        v2.44
  |
  v
ROUTER                   v2.43
  |
  v
RELIABLE LINK            v2.42
  |
  v
FRAME + CRC
  |
  v
PIO PHY
```

O teste v2.44 ainda é determinístico e local ao firmware de teste: ele comprova a decisão e a evolução do cabeçalho sem alegar que três Pico já estejam encaminhando fisicamente o frame.
