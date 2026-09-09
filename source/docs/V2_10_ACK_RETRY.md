# T425-SIM v2.10 — ACK / timeout / retry

A v2.10 acrescenta uma máquina de estados de confirmação no transmissor.

```text
IDLE
 │
 │ send DATA(seq)
 ▼
WAIT_ACK
 │
 ├── ACK(seq) ─────────► IDLE
 │
 └── timeout
       │
       ▼
     RETRY
       │
       ├── retry < MAX ─► WAIT_ACK
       │
       └── retry == MAX ► FAILED
```

O ACK é um frame T425 com:

- `TYPE = 0x06`
- `SEQ = sequência confirmada`
- `LEN = 0`

O timeout é representado por ticks monotônicos. A integração do tick com o timer real do RP2040 será feita na etapa de firmware.
