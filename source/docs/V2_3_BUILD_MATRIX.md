# Matriz de build

Quatro imagens podem ser geradas do mesmo código:

| Nó | T425_NODE_ID |
|---|---:|
| P00 | 0 |
| P01 | 1 |
| P02 | 2 |
| P03 | 3 |

A configuração do nó deve ser feita no build, evitando duplicação do firmware.

## Critério de aceitação

Cada Pico deve inicializar por USB e anunciar:

`T425 Pn READY`

antes da ativação do enlace físico.
