# v2.2 — Bring-up de hardware

## Sequência

```text
PC/Mac
  │ USB
  ▼
P00 ───── P01
       │
       └──── adicionar P02
                  │
                  └──── adicionar P03
```

## Critérios

### Etapa A — loopback
TX deve retornar para RX sem perda de bytes.

### Etapa B — ponto a ponto
Enviar frames conhecidos e validar SOF, comprimento, CRC quando introduzido, SRC/DST e SEQ.

### Etapa C — reliable link
Forçar perda/timeout e confirmar retransmissão.

### Etapa D — rendezvous
Executar RDV_REQ/RDV_ACK.

### Etapa E — multi-hop
Executar P00→P01→P02→P03.

## Observação
CRC/FCS ainda deve ser incorporado ao frame físico antes de considerar o enlace robusto para cabos maiores ou ambiente ruidoso.
