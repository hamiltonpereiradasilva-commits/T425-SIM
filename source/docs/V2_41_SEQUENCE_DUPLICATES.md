# T425-SIM v2.41 — sequência, duplicatas e ACK idempotente

A v2.41 fecha uma falha conceitual que não pode ficar para depois: retransmissão pode gerar
duplicação se o DATA foi recebido mas o ACK foi perdido.

## Regra

Para cada DATA válido:

1. validar frame/CRC;
2. verificar `SEQ`;
3. entregar ao nível superior somente se for a próxima sequência;
4. enviar ACK mesmo quando DATA for duplicado;
5. nunca entregar duas vezes o mesmo `SEQ`.

Assim, o caso:

DATA(seq=7) -> recebido
ACK(seq=7) -> perdido
DATA(seq=7) -> retransmitido

resulta em:

```text
primeiro DATA   -> entrega
ACK             -> perdido
retransmissão   -> DUPLICATE, sem nova entrega
ACK             -> enviado novamente
```

## Contadores

```text
DUP = DATA duplicado
OOO = DATA fora de ordem
```

A versão ainda usa stop-and-wait, portanto OOO deve ser zero em operação normal.
