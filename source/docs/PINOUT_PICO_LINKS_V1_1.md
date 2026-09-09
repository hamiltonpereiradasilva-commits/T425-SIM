# T425 RP2040 — plano de links físicos v1.1

O projeto usa dois links por nó na topologia em anel: LEFT e RIGHT.

| Nó | LEFT RX/TX | RIGHT RX/TX |
|---|---:|---:|
| P00 | GP2/GP3 | GP4/GP5 |
| P01 | GP2/GP3 | GP4/GP5 |
| P02 | GP2/GP3 | GP4/GP5 |
| P03 | GP2/GP3 | GP4/GP5 |

**Importante:** esta é uma proposta lógica de pinagem, não uma confirmação da placa/carrier específica. Antes de conectar fisicamente, conferir o pinout exato do Pico utilizado e os níveis elétricos.

Ligação full-duplex proposta:

```text
Node A TX ─────────► Node B RX
Node A RX ◄───────── Node B TX
GND      ─────────── Node B GND
```

Para o primeiro bring-up, utilizar apenas dois nós e cabo curto.
