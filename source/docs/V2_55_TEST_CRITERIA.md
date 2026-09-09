# V2.55 — critérios de aceitação

## Teste A — rota primária

Esperado:

```text
NEXT=P01
IF=0
```

## Teste B — interface primária DOWN

Esperado:

```text
NEXT=P02
IF=1
```

## Teste C — todas as interfaces candidatas DOWN

Esperado:

```text
NO_USABLE_ROUTE
```

## Teste D — recuperação

Após restabelecer IF0:

```text
NEXT=P01
IF=0
```

## Regra de segurança

Nunca executar:

```text
route_select() -> NEXT_HOP -> transmit()
```

sem validar:

```text
interface_for_peer(NEXT_HOP)
```

e o estado UP da interface.
