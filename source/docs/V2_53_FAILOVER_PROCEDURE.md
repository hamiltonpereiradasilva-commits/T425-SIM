# V2.53 — procedimento de validação física

## Cabos

```text
P00 <-> P01
P00 <-> P02
P01 <-> P03
P02 <-> P03
```

Cada enlace deve ter TX, RX e GND conforme o esquema elétrico adotado.

## Teste normal

Enviar:

```text
TX_ID=100
SRC=0
FINAL_DST=3
```

Esperado:

```text
P00 -> P01 -> P03
DELIVERED
```

## Falha do caminho primário

Interromper apenas:

```text
P00 <-> P01
```

O estado do peer P01 deve chegar a DOWN.

A próxima transmissão para P03 deve selecionar:

```text
P02
```

e chegar por:

```text
P00 -> P02 -> P03
```

## Falha total

Interromper também:

```text
P00 <-> P02
```

Resultado:

```text
NO_ROUTE
```

e a transação E2E não pode ser marcada como DELIVERED.

## Recuperação

Restabelecer P00-P01.

Depois de ACK/health válido:

```text
P01 = UP
```

A rota de menor métrica volta a ser candidata primária.

## Critérios

O teste só é considerado aprovado se:

1. o tráfego realmente mudar de enlace;
2. P03 receber apenas uma entrega da aplicação;
3. TX_ID permanecer igual durante o failover;
4. SEQ continuar sendo hop-local;
5. a origem não declarar DELIVERED antes da confirmação E2E.
