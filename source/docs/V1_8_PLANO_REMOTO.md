# Próxima extensão: rendezvous remoto

Para um canal distribuído:

```text
P00                                  P01
PROC A                               PROC B
  │                                    │
  │ OUT(c,x)                           │
  │                                    │
  ├──── RDV_REQ(chan,seq,x) ─────────►│
  │                                    │
  │                         IN(c,x)    │
  │                                    │
  │◄──── RDV_ACK(chan,seq) ───────────│
  │                                    │
  ▼                                    ▼
continua                             continua
```

O transporte deverá reutilizar o reliable link existente, sem duplicar a lógica de ACK/retry.
