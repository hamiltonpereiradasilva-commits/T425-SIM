# T425-SIM v2.27 — one-hop timed validation

A v2.27 corrige o último problema do harness: o tempo de recepção/transmissão deixa de ser implícito.

Cada evento recebe `now` explicitamente.

## Sequência validada

```text
t=100  P00 ──DATA(seq=0)──► P01
t=100  P00 ◄────ACK(seq=0)─ P01

t=200  P00 ──DATA(seq=1)──► P01
       ACK perdido

t=209  P00 permanece WAIT_ACK

t=210  timeout
       WAIT_ACK → RETRY

t=210  retransmite DATA(seq=1)

t=211  P00 ◄────ACK(seq=1)─ P01
       RETRY → IDLE

t=212  CRC inválido
       frame rejeitado
```

O objetivo é validar a máquina de estados antes de conectar o driver PIO real.
