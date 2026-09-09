# v1.4 — Primeiro protocolo de bancada

A v1.4 acrescenta controle de link sobre o framing.

## Estados

```text
IDLE → PING_SENT → PONG_RECEIVED
                    │
                    └── DATA
```

## Primeiro teste

P00 envia `PING` para P01. P01 responde `PONG` usando o mesmo `SEQ`.

Depois P00 envia `DATA`.

## Firmware

`firmware_pico/main.c` pode ser compilado para cada nó definindo:

`-DT425_NODE_ID=0` para P00
`-DT425_NODE_ID=1` para P01

## Observação

O firmware entregue é um bring-up inicial. O RX/TX ainda deve ser instrumentado e validado em bancada antes de transformar o teste em uma implementação de produção.
