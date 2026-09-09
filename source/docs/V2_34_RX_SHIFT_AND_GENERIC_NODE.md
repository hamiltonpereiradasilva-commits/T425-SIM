# T425-SIM v2.34 — RX shift correction + firmware genérico

Antes de transformar o projeto em uma rede de N nós, foi corrigido um ponto crítico no RX PIO: o sentido de deslocamento do ISR.

## Correção

O RX agora configura:

```c
sm_config_set_in_shift(&cr, false, true, 8);
```

ou seja:

- shift-left;
- autopush;
- threshold de 8 bits.

Isso torna a representação recebida coerente com a transmissão LSB-first e com a leitura de um byte pelo FIFO.

O TX também declara explicitamente seu shift:

```c
sm_config_set_out_shift(&ct, true, false, 32);
```

## Firmware único

Foi adicionada a configuração:

```text
T425_NODE_ID
T425_NODE_COUNT
T425_TX_PIN
T425_RX_PIN
```

Portanto P00, P01, P02 e P03 usam o mesmo código-fonte. A identidade é uma definição de compilação/configuração:

```text
P00 → NODE_ID=0
P01 → NODE_ID=1
P02 → NODE_ID=2
P03 → NODE_ID=3
```

A mesma regra permite P04...Pnn sem criar um firmware diferente.

## Estado

Esta correção é estática e de integração de fonte. A validação física do RX continua pendente do teste em dois Pico.
