# v2.9 — Integração PIO/FIFO

A integração prevista é:

```text
PIO RX
  │
  ▼
RX FIFO
  │
  ▼
t425_stream_push(byte)
  │
  ├── incomplete → continua
  │
  └── complete → t425_link_stack_receive()
```

TX:

```text
t425_link_stack_prepare()
        │
        ▼
TX buffer
        │
        ▼
PIO TX FIFO
```

Nesta entrega o parser está validado no host. A leitura/escrita efetiva das FIFOs do PIO ainda deve ser feita no firmware RP2040 e medida no hardware.
