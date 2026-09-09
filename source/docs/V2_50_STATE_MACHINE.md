# v2.50 — estados observáveis

Por transação E2E:

```text
IDLE
  |
  v
PENDING
  |
  +----> DELIVERED
  |
  +----> FAILED
```

Por salto, o Reliable Link possui sua própria atividade de transmissão e confirmação.

A combinação correta é:

```text
E2E PENDING
   +
hop ACKs locais
   +
route forwarding
   +
final delivery
   =
E2E DELIVERED
```
