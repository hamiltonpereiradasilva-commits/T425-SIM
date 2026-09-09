# T425-SIM v2.52 — múltiplos next-hops e failover

A v2.52 altera o modelo de roteamento de:

```text
DESTINO -> NEXT_HOP
```

para:

```text
DESTINO -> [NEXT_HOP primário, NEXT_HOP alternativo, ...]
```

Cada candidato possui um `metric`.

O seletor escolhe o candidato válido de menor métrica cujo peer esteja UP.

## Exemplo

Para FINAL_DST=3:

```text
primário:    P02 metric=2
alternativo: P01 metric=3
```

Normal:

```text
P02 UP -> NEXT=P02
```

Falha:

```text
P02 DOWN
P01 UP
       ↓
NEXT=P01
```

Se ambos estiverem DOWN:

```text
NO_ROUTE
```

Quando P02 recuperar:

```text
P02 UP -> NEXT=P02
```

## Observação de topologia

A cadeia:

```text
P00-P01-P02-P03
```

não fornece fisicamente um segundo caminho.

O failover só pode ser validado em hardware depois que for adicionada uma ligação
alternativa. A v2.52 valida primeiro a política de seleção.
