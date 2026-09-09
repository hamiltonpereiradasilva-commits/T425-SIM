# T425-SIM v2.9 — Stream parser

A v2.9 resolve a diferença entre **FIFO de bytes** e **frames**.

O PIO entrega bytes; o parser reconstrói:

```text
PIO RX FIFO
    │
    ▼
byte stream
    │
    ▼
SOF detector
    │
    ▼
5-byte header
    │
    ▼
payload LEN
    │
    ▼
CRC16
    │
    ▼
complete frame
```

O parser tolera bytes fora de frame procurando novamente `SOF`.

Estados:

- `WAIT_SOF`
- `HEADER`
- `PAYLOAD`
- `CRC`

A validação do CRC é feita reutilizando `t425_frame_decode()`.
