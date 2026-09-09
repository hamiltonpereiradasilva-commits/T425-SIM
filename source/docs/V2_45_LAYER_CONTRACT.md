# Contrato entre camadas

```text
FORWARD ENGINE
    |
    | next_hop + payload
    v
RELIABLE LINK
    |
    | frame DATA/ACK
    v
PIO PHY
```

O Reliable Link continua hop-by-hop.

O endereço final permanece no cabeçalho de forwarding.

Assim, em P01:

```text
FINAL_DST = 2
NEXT_HOP  = 2
```

e o Reliable Link apenas garante a entrega do salto P01 -> P02.

Essa distinção será preservada na implementação física.
