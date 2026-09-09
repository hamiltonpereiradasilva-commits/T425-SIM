# T425-SIM v2.55 — integração Router + Interface Manager + Link

A v2.55 integra pela primeira vez as três decisões:

```text
1. qual NEXT_HOP usar?
2. qual interface física corresponde ao NEXT_HOP?
3. essa interface está utilizável?
```

Fluxo:

```text
FINAL_DST
    |
    v
ROUTE_SELECT
    |
    v
NEXT_HOP
    |
    v
INTERFACE_FOR_PEER
    |
    v
IF_ID
    |
    v
PIO/PHY
```

## Exemplo P00

```text
IF0 -> P01 -> PIO SM0 -> GPIO 2/3
IF1 -> P02 -> PIO SM1 -> GPIO 4/5
```

Rotas para P03:

```text
P01 metric=2
P02 metric=2
```

Em condição normal, P01 é escolhido por ser o primeiro candidato de mesma métrica.

Se IF0 estiver DOWN, P01 deixa de ser utilizável e P02 é selecionado.

Se IF0 e IF1 estiverem DOWN:

```text
NO_USABLE_ROUTE
```

O ponto importante é que uma rota configurada não basta: o Router só pode
transmitir quando existe uma interface física UP correspondente ao next-hop.
