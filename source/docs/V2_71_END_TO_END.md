# V2.71 — caminho lógico completo

```text
MacBook
  │
  ▼
USB frame
  │
  ▼
parser
  │
  ▼
SET_CONFIG
  │
  ▼
staging RAM
  │
  ▼
COMMIT
  │
  ▼
canonical validation
  │
  ▼
A/B Flash transaction
  │
  ▼
ACK
```

O caminho está logicamente fechado.

A camada física do RP2040 continua sendo o último gate de hardware.
