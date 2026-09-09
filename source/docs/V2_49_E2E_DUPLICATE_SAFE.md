# T425-SIM v2.49 — E2E físico e supressão de duplicatas

A v2.49 trata o caso crítico:

```text
P00 -> P01 -> P02 -> P03
                     |
                     | entrega
                     v
                  ACK E2E
```

Se o ACK E2E for perdido, P00 pode retransmitir a transação.

Isso pode fazer o mesmo `TX_ID` chegar novamente a P03.

## Regra

O destino mantém a identidade da última transação entregue:

```text
LAST_TX_ID
VALID
```

Se:

```text
TX_ID == LAST_TX_ID
```

a segunda chegada não é entregue novamente à aplicação.

Ela pode, porém, gerar novamente a confirmação E2E necessária para que a origem
saia de PENDING.

## Resultado desejado

```text
primeira chegada:
TX_ID=42 -> ACCEPT=1

ACK perdido

segunda chegada:
TX_ID=42 -> ACCEPT=0

origem:
PENDING -> DELIVERED
```

Assim conseguimos simultaneamente:

- at-least-once no transporte lógico;
- exactly-once na entrega à aplicação dentro do escopo da janela de deduplicação;
- ACK E2E repetível.

## Limite

A tabela de deduplicação desta versão guarda apenas uma identidade. Uma implementação
completa deverá usar uma janela ou cache de TX_IDs para lidar com reordenação e
reinicialização.
