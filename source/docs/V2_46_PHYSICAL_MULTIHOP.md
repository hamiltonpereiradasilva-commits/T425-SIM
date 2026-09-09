# T425-SIM v2.46 — integração física multi-hop

Esta versão substitui o callback lógico da v2.45 por uma função que transmite
através do Reliable Link/PIO PHY.

## Topologia

```text
P00 <----> P01 <----> P02
```

Cada Pico usa o mesmo firmware. Apenas `T425_NODE_ID` muda no build.

## Encaminhamento

P00 cria:

```text
SRC=0
FINAL_DST=2
DST=1
```

P01:

1. recebe o DATA P00->P01;
2. valida CRC;
3. aplica controle de sequência;
4. envia ACK para P00;
5. consulta a rota FINAL_DST=2;
6. muda DST para 2;
7. decrementa TTL;
8. incrementa HOP_COUNT;
9. cria um novo DATA P01->P02.

P02:

1. recebe o DATA P01->P02;
2. valida CRC;
3. envia ACK para P01;
4. reconhece FINAL_DST=2;
5. entrega.

## Limitação importante

Esta versão implementa o primeiro caminho físico multi-hop no código, mas o pacote
não deve ser considerado "validado" sem executar o teste com três Pico e observar
os logs USB dos três nós.

## Questão deliberadamente não resolvida

O ACK de cada salto confirma somente o salto local. A garantia fim-a-fim ainda será
objeto de uma camada superior, se necessária.

## Fiação

Para cada enlace UART/PIO:

```text
P00 GP2 TX -> P01 GP3 RX
P00 GP3 RX <- P01 GP2 TX
GND        -> GND

P01 GP2 TX -> P02 GP3 RX
P01 GP3 RX <- P02 GP2 TX
GND        -> GND
```

Os três Pico podem permanecer conectados individualmente ao MacBook por USB para
telemetria. Não se deve usar os sinais USB como transporte entre os Pico.
