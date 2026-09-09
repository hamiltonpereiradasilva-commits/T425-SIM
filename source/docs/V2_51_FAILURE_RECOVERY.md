# T425-SIM v2.51 — detecção de falha de enlace

A v2.51 introduz uma camada explícita de saúde do enlace.

Estados:

```text
UP -> DEGRADED -> DOWN
```

Uma confirmação válida recupera o enlace:

```text
DOWN -> UP
```

na máquina de saúde desta etapa.

## Regra

Um timeout isolado não significa necessariamente que o enlace morreu.

Com três timeouts consecutivos:

```text
TIMEOUT 1 -> DEGRADED
TIMEOUT 2 -> DEGRADED
TIMEOUT 3 -> DOWN
```

O Router deve deixar de tratar aquele próximo salto como utilizável e solicitar
uma rota alternativa.

## E2E

Se:

```text
P02 -> P03
```

ficar DOWN durante:

```text
P00 -> P01 -> P02 -> P03
```

a transação E2E permanece PENDING/FAILURE, nunca DELIVERED.

## Escopo

Esta versão implementa a máquina de saúde e o teste de transição. A seleção
dinâmica de uma rota alternativa é a próxima etapa.
