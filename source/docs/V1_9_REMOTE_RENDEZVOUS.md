# T425-SIM v1.9 — Rendezvous remoto

A v1.9 leva o rendezvous ao enlace entre RP2040.

```text
P00                              P01
 │                                │
 │ RDV_REQ(chan,seq,data)         │
 ├───────────────────────────────►│
 │                                │
 │                    entrega ao IN│
 │                                │
 │◄──────── RDV_ACK(chan,seq) ────│
 │                                │
 ▼                                ▼
continua                         continua
```

Tipos reservados:
- `RDV_REQ = 0x20`
- `RDV_ACK = 0x21`
- `RDV_CANCEL = 0x22`

A camada mantém `node_id`, `seq`, canal e destino pendentes.

A confirmação de transporte continua sendo responsabilidade do reliable link. O ACK do rendezvous confirma a operação lógica no canal; são semânticas diferentes e devem permanecer separadas.
