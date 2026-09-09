# v2.46 — build e execução

## Três instâncias

Usar o mesmo firmware com:

```text
T425_NODE_ID=0
T425_NODE_ID=1
T425_NODE_ID=2
```

O executável de teste é:

```text
t425_multihop_physical.c
```

A CMake deve incluir, no mínimo:

```text
t425_multihop_physical.c
t425_pio_phy.c
t425_pio_phy.pio
t425_reliable_link.c
t425_crc16.c
t425_router.c
t425_forward.c
```

e os headers correspondentes.

## Critério de sucesso

P00:

```text
TX > 0
ACKED > 0
```

P01:

```text
RX > 0
FWD > 0
ACKED > 0, quando também originar tráfego
```

P02:

```text
RX > 0
DELIVER > 0
```

Em operação normal:

```text
CRC=0
DUP=0
OOO=0
TIMEOUT=0
```

Pequenos RETX podem ocorrer em uma bancada real.

## Não confundir

USB -> MacBook é somente console/telemetria.

O caminho de dados do experimento é:

```text
GPIO/PIO -> GPIO/PIO
```
