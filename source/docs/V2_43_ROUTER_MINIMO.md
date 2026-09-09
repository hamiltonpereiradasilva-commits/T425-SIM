# T425-SIM v2.43 — Router mínimo

A v2.43 introduz a primeira camada de roteamento acima do Reliable Link.

## Princípio

O Router **não conhece PIO, UART, CRC ou temporização**.

Ele recebe apenas uma decisão lógica:

```text
SRC -> DST
```

e responde:

```text
LOCAL_DELIVER
FORWARD
DROP
```

Para FORWARD também fornece:

```text
NEXT_HOP
```

## Tabela

Cada nó possui uma tabela simples:

```text
DESTINATION | NEXT_HOP | VALID
```

A API aceita até 16 nós nesta primeira implementação.

## Topologia inicial

Apenas dois nós:

```text
P00 <--------> P01
```

Rotas:

```text
P00: DST 1 -> HOP 1
P01: DST 0 -> HOP 0
```

DST 2 ainda não possui rota e deve resultar em DROP.

## Separação de camadas

```text
OCCAM
  |
  v
ROUTER                 <- v2.43
  |
  v
RELIABLE LINK          <- v2.42
  |
  v
FRAME / CRC
  |
  v
PIO PHY
```

Não foi introduzido ainda encaminhamento físico de um frame entre três ou quatro nós.
Primeiro validamos a decisão de roteamento isoladamente.
