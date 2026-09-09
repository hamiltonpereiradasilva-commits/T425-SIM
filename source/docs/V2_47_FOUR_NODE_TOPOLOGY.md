# T425-SIM v2.47 — topologia física de quatro nós

A v2.47 prepara o salto de três para quatro Pico mantendo **um único firmware**.

## Topologia

```text
P00 <-> P01 <-> P02 <-> P03
```

Rotas principais:

```text
P00: 1->1, 2->1, 3->1
P01: 0->0, 2->2, 3->2
P02: 0->1, 1->1, 3->3
P03: 0->2, 1->2, 2->2
```

Portanto, por exemplo:

```text
P00 -> P03:
0 -> 1 -> 2 -> 3

P03 -> P00:
3 -> 2 -> 1 -> 0
```

## O que esta versão prova

A tabela de roteamento pode ser parametrizada pelo `NODE_ID`, sem criar um firmware
diferente para cada Pico.

## O que ainda não prova

Não declara a rede de quatro nós fisicamente validada. O teste físico deve ser feito
com os quatro Pico e seus consoles USB independentes.

## Próxima etapa

Integrar o motor de forwarding físico da v2.46 com a topologia de quatro nós e
testar uma mensagem P00 -> P03 atravessando três enlaces.
