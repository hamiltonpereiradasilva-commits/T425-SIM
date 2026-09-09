# T425-SIM v2.11 — Generic Node Firmware

A arquitetura deixa de assumir que existem somente P00..P03.

O mesmo código de runtime é utilizado por qualquer nó compatível:

```text
                 T425 Generic Firmware
                         │
              ┌──────────┴──────────┐
              │                     │
           NODE_ID              LINK TABLE
              │                     │
              └──────────┬──────────┘
                         ▼
                  Node Manager
                         │
                  Router / Links
```

## Configuração

Cada nó possui:

- `node_id`;
- número de links;
- vizinho de cada link;
- TX/RX GPIO;
- PIO;
- state machines TX/RX;
- baud rate.

O código não contém uma árvore fixa `if NODE_ID == 0/1/2/3`.

A topologia P00-P03 é apenas uma configuração de bancada.

## Escalabilidade

O limite lógico da tabela é 255 IDs, enquanto o número de links físicos por nó nesta implementação é 4. A quantidade real de nós dependerá da topologia, da largura de banda, memória, alimentação e camada de gerenciamento.
