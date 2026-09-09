# T425-SIM v2.45 — motor de forwarding multi-hop

A v2.45 fecha a primeira integração lógica entre Router e cabeçalho de forwarding.

## Fluxo validado

```text
P00 -> P01 -> P02
```

O mesmo pacote lógico percorre:

```text
P00: FORWARD -> HOP 1
P01: FORWARD -> HOP 2
P02: LOCAL_DELIVER
```

## O que foi acrescentado

` t425_forward_engine.* ` separa a decisão de encaminhamento da PHY.

O motor:

1. verifica validade do cabeçalho/TTL;
2. consulta o Router usando `FINAL_DST`;
3. entrega localmente se o nó for o destino;
4. consulta `NEXT_HOP` para forwarding;
5. incrementa `HOP_COUNT`;
6. decrementa `TTL`;
7. chama uma função de envio para o próximo salto.

## Limite desta versão

O teste incluído é de integração lógica. A função de envio ainda é um callback de teste;
portanto esta versão **não deve ser apresentada como teste físico P00-P01-P02**.

O próximo passo é substituir o callback pelo envio real através do Reliable Link,
mantendo a mesma interface de forwarding.
