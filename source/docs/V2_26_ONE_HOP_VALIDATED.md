# T425-SIM v2.26 — One-hop validation harness

A v2.26 corrige o modelo de teste da v2.25 separando os sentidos de transmissão.

## Modelo

```text
P00 TX ───────── DATA ─────────► P01 RX
P00 RX ◄──────── ACK ─────────── P01 TX
```

Cada callback representa uma direção física.

## Casos validados

1. DATA → ACK → `IDLE`
2. ACK perdido → `WAIT_ACK`
3. timeout → `RETRY`
4. recuperação do pacote pendente com a mesma `SEQ`
5. retransmissão
6. ACK restaurado → `IDLE`
7. CRC inválido → rejeição

Este teste valida a máquina de transporte de um salto sem ainda fingir que a PIO foi exercitada no hardware.

O próximo passo é substituir `channel_tx()` pelo driver PIO RP2040.
