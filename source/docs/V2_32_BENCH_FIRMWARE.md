# T425-SIM v2.32 — firmware de bancada PIO

Esta versão cria o primeiro firmware mínimo de bancada.

## Objetivo

Não há Router, OCCAM ou Reliable Link nesta imagem. O objetivo é exclusivamente observar:

- inicialização do PIO;
- clock real do sistema;
- transmissão de padrão conhecido;
- recepção;
- contadores TX/RX/ERR pelo USB CDC.

## Padrão

```text
55 AA 00 FF 12 34 C3 3C
```

## USB

O firmware imprime:

```text
T425_BENCH V2.32
PIO=0 TX=2 RX=3 BAUD=115200 SYS_HZ=...
PHY_INIT=OK
STAT TX=... RX=... ERR=...
```

## Bancada

Dois Pico:

```text
P00 GP2 TX ─────────► P01 GP3 RX
P00 GP3 RX ◄───────── P01 GP2 TX
GND      ──────────── GND
```

Cada Pico deve usar a mesma imagem, com a mesma pinagem.

## Critério

A primeira validação deve ser visual no analisador lógico. Em seguida, verificar RX/TX no USB.

Esta versão é software-preparada; a validação elétrica continua dependente dos Pico físicos.
