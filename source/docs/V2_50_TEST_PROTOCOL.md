# Protocolo de teste físico v2.50

## Fase A — um sentido

```text
P00 -> P03
```

Esperado:

```text
P00 -> P01 -> P02 -> P03
```

## Fase B — sentido inverso

```text
P03 -> P00
```

Esperado:

```text
P03 -> P02 -> P01 -> P00
```

## Fase C — cruzado

```text
P01 -> P03
P02 -> P00
```

## Fase D — bidirecional

Intercalar:

```text
P00 -> P03
P03 -> P00
P01 -> P03
P02 -> P00
```

## Critérios

Cada transação deve manter:

```text
TX_ID
SRC
FINAL_DST
```

e cada salto deve manter sua própria sequência/ACK.

Não declarar sucesso apenas porque um ACK local apareceu.
O critério final é `DELIVERED` no destino correto.
