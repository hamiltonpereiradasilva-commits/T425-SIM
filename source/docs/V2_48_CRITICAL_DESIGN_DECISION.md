# Decisão de projeto v2.48

O T425 terá duas semânticas distintas:

```text
LINK ACK
```

e

```text
END-TO-END DELIVERY
```

Não será permitido usar um ACK hop-by-hop como confirmação de entrega final.

Isso evita mascarar falhas posteriores no caminho.

## Consequência

A camada Router pode permanecer simples:

```text
FINAL_DST -> NEXT_HOP
```

O Reliable Link pode permanecer local:

```text
DATA -> ACK
```

A camada E2E acompanha:

```text
TX_ID
SRC
FINAL_DST
STATE
ATTEMPTS
```

Essa separação deixa o sistema extensível para redes maiores.
