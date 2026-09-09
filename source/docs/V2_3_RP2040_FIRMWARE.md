# T425-SIM v2.3 — Firmware RP2040

A v2.3 inicia a transição do modelo de host para o firmware executável no RP2040.

## Separação de planos

```text
                 RP2040
        ┌─────────────────────┐
        │ OCCAM / Runtime     │
        │ Router / RDV        │
        │ Reliable Link       │
        │ PIO                 │
        ├─────────────────────┤
        │ USB CDC diagnóstico │
        └──────────┬──────────┘
                   │
                MacBook
```

USB é reservado para console/diagnóstico. O tráfego entre nós será tratado pelos PIOs.

## Estado

O firmware desta entrega:
- compila como projeto Pico SDK quando o SDK estiver disponível;
- inicializa stdio;
- apresenta `P00..P03` por macro `T425_NODE_ID`;
- fornece o ponto de entrada para integração do PIO.

A temporização física do PIO e o driver de frames ainda precisam de validação no hardware.
