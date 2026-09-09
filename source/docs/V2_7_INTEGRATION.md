# Integração v2.7

## Caminho TX

1. runtime cria payload;
2. reliable link atribui SEQ;
3. frame builder cria frame;
4. CRC16 é anexado;
5. buffer TX recebe bytes;
6. PIO transmite.

## Caminho RX

1. PIO deposita bytes;
2. parser identifica frame;
3. CRC é verificado;
4. frame válido incrementa RX;
5. reliable link trata SEQ/ACK;
6. router decide entrega local ou próximo salto.

## Próxima integração

A v2.8 deve ligar explicitamente:
`PIO FIFO -> t425_link_stack_receive()`
e
`t425_link_stack_prepare() -> PIO FIFO`.
