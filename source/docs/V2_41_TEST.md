# Teste v2.41

## T1 — operação normal

Esperado:

```text
DUP=0
OOO=0
RETX=0
TIMEOUT=0
```

## T2 — perda de ACK

Forçar perda de um ACK.

Esperado:

```text
RETX > 0
DUP > 0
```

mas a entrega ao nível superior deve ocorrer uma única vez.

## T3 — corrupção

Usar a injeção da v2.40.

Esperado:

```text
CRC > 0
RETX > 0
```

## T4 — sequência fora de ordem

Injetar manualmente um DATA com SEQ diferente do esperado.

Esperado:

```text
OOO > 0
```

## Ligações

P00 GP2 TX -> P01 GP3 RX
P00 GP3 RX <- P01 GP2 TX
GND -> GND
