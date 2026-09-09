# T425-SIM v1.8 — Rendezvous síncrono

A v1.8 introduz uma máquina de estados para sincronização direta entre `OUT` e `IN`.

```text
EMPTY
 │
 ├── OUT ──► WAIT_OUT ── IN ──► TRANSFER
 │
 └── IN  ──► WAIT_IN  ── OUT ─► TRANSFER
                              │
                              ▼
                            DONE
```

O produtor e o consumidor só completam a operação quando ambos estão presentes.

Esta implementação é o núcleo local do rendezvous. O próximo passo é adicionar identificação do canal e encapsulamento no protocolo remoto, para que o mesmo mecanismo opere entre P00/P01/P02/P03.
