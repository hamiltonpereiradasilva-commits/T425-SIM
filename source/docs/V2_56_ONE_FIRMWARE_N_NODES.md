# T425-SIM v2.56 — um firmware, N nós

A v2.56 formaliza a decisão de arquitetura:

> O protocolo não deve conter lógica específica para P00, P01, P02 ou P03.

O firmware comum recebe uma configuração de inicialização:

```text
NODE_ID
INTERFACE_TABLE
```

A tabela informa somente:

```text
PEER_ID
PIO_SM
TX_GPIO
RX_GPIO
```

## Consequência

O mesmo firmware pode ser carregado em:

```text
P00
P01
P02
P03
...
PN
```

A diferença entre os nós está nos dados de configuração, não em cópias diferentes
da lógica do protocolo.

## Limite real

Isso não significa que qualquer número de nós caiba em um único Pico.

O firmware pode ser genericamente N-nodes, mas cada nó continua limitado por:

- quantidade de interfaces físicas;
- número de PIO state machines;
- GPIO disponíveis;
- memória RAM/flash;
- tamanho das tabelas de rotas e vizinhança;
- capacidade de processamento;
- largura de banda dos enlaces.

Portanto:

```text
mesmo firmware != recursos físicos infinitos
```

## Estratégias de configuração

Há três possibilidades futuras:

1. configuração compilada em uma pequena área de configuração;
2. configuração gravada em flash;
3. configuração recebida por USB/console no boot.

A lógica de protocolo permanece a mesma nas três.
