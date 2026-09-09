# Integração v1.9

```text
OCCAM OUT
   │
   ▼
local rendezvous
   │
   ▼
remote RDV_REQ
   │
   ▼
reliable link
   │
   ▼
PIO
   │
   ▼
RP2040 remoto
   │
   ▼
remote channel
   │
   ▼
IN
   │
   ▼
RDV_ACK
```

A próxima etapa deve conectar a máquina de estados do rendezvous ao scheduler, de forma que um processo OCCAM bloqueie em `OUT`/`IN` e seja liberado somente pela conclusão do rendezvous local ou remoto.
