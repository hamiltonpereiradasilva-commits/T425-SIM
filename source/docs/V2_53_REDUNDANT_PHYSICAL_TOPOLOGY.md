# T425-SIM v2.53 — topologia redundante física

A v2.53 define uma topologia de quatro nós com dois caminhos fisicamente
independentes entre P00 e P03:

```text
             P01
            /   \
           /     \
         P00     P03
           \     /
            \   /
             P02
```

Enlaces:

```text
P00 <-> P01
P00 <-> P02
P01 <-> P03
P02 <-> P03
```

Caminhos:

```text
A: P00 -> P01 -> P03
B: P00 -> P02 -> P03
```

## Objetivo

Em condição normal:

```text
P00 -> P01 -> P03
```

Se o enlace P00-P01 ficar DOWN:

```text
P00 -> P02 -> P03
```

deve ser selecionado.

## Observação de hardware

A rede deixa de ser linear. Portanto, cada Pico precisa de pelo menos dois
enlaces físicos independentes nos nós intermediários.

A implementação final do driver deverá associar cada peer a uma interface
PIO/UART física. A tabela de rotas não deve esconder essa associação.
