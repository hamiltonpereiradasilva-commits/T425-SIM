# V2.51 — matriz de falhas

## Teste 1 — enlace saudável

```text
P00 -> P01 -> P02 -> P03
```

Esperado:

```text
UP
DELIVERED
```

## Teste 2 — perda temporária

Um ou dois timeouts consecutivos.

Esperado:

```text
DEGRADED
```

e recuperação após ACK.

## Teste 3 — perda persistente

Três timeouts consecutivos no mesmo próximo salto.

Esperado:

```text
DOWN
```

e:

```text
ROUTE_ACTION=REQUIRE_ALTERNATIVE
```

## Teste 4 — falha após ACKs anteriores

O enlace P02-P03 pode funcionar normalmente e depois falhar.

O histórico anterior de ACK não pode mascarar a falha atual.

## Teste 5 — recuperação

Após o enlace voltar:

```text
ACK -> UP
```

A rota poderá voltar a ser utilizável após a política de recuperação.
