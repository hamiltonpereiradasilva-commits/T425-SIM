# T425-SIM v2.38 — primeiro enlace lógico P00↔P01

A v2.38 sobe uma camada acima da PHY.

A PHY continua baseada nos exemplos oficiais de PIO UART da Raspberry Pi: TX 8N1 a 1 bit por 8 ciclos PIO e RX com autopush em 8 bits. citeturn0search1turn0search2

## Novo

Foi acrescentado um frame mínimo:

SYNC | VERSION | SRC | DST | SEQ | LEN | PAYLOAD | CRC16

com CRC-16/CCITT.

## Mesmo firmware

P00:
T425_NODE_ID=0
T425_PEER_ID=1

P01:
T425_NODE_ID=1
T425_PEER_ID=0

O código-fonte é o mesmo. Apenas a configuração muda.

## Teste

P00 gera frames de 32 bytes e P01 os devolve.

P00 conta:
TX, RX, CRC, DST, SEQ.

Isso é um teste de enlace, ainda não é o Reliable Link final.

## Próxima etapa

Após comprovar P00↔P01:
- timeout;
- ACK;
- retransmissão;
- janela/stop-and-wait;
- depois Router;
- depois OCCAM.
