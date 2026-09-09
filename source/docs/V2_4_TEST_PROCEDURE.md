# Procedimento de teste v2.4

## 1 — USB
Gravar cada nó e confirmar:
`USB CDC READY`
`PIO LINK DRIVER READY (bring-up)`

## 2 — PIO isolado
Executar TX/RX no mesmo Pico em loopback.

## 3 — ponto a ponto
Conectar somente P00↔P01.

## 4 — integridade
Enviar sequência:
`00 01 02 ... FE FF`
e comparar byte a byte.

## 5 — erros
Inserir perda/atraso no teste de host antes de depender do hardware.

## 6 — reliable link
Somente depois do loopback físico integrar ACK, timeout, retransmissão e CRC.
