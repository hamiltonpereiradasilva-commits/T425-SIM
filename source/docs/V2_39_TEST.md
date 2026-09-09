# Teste v2.39

P00:
NODE_ID=0
PEER_ID=1

P01:
NODE_ID=1
PEER_ID=0

Ligação:
P00 GP2 -> P01 GP3
P00 GP3 <- P01 GP2
GND -> GND

Esperado em condições normais:
TX cresce em P00
RX cresce em ambos
ACK cresce em P00
RETX = 0
TIMEOUT = 0

Depois interromper temporariamente o fio TX de P01 para P00:
RETX deve crescer.
Após MAX_RETRY, TIMEOUT deve crescer.

Isso valida a máquina de estados do Reliable Link.
