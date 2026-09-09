# T425-SIM v2.48 — confiabilidade fim-a-fim

A v2.48 resolve conceitualmente a distinção que aparece quando uma mensagem atravessa
vários enlaces.

## Não confundir

Um ACK:

```text
P00 -> P01
```

significa apenas:

```text
P01 recebeu corretamente o frame daquele salto.
```

Ele **não** significa:

```text
P03 recebeu a mensagem da aplicação.
```

## Dois níveis

### Reliable Link

```text
P00 -> P01
```

garante o salto local.

Depois:

```text
P01 -> P02
```

é outra transação.

Depois:

```text
P02 -> P03
```

é outra.

### E2E

A aplicação de P00 mantém uma transação lógica:

```text
TX_ID=42
SRC=0
FINAL_DST=3
```

Essa transação só passa para:

```text
DELIVERED
```

quando P03 confirma a entrega final através do mecanismo fim-a-fim.

## Por que isso é necessário?

Cenário:

```text
P00 -> P01   OK
P01 -> P02   OK
P02 -> P03   FALHA
```

P00 não pode considerar a operação concluída apenas porque P00->P01 recebeu ACK.

## Escopo desta versão

O módulo `t425_e2e.*` formaliza a máquina de estado mínima:

```text
IDLE
  |
  v
PENDING
  |   |  \ max attempts
  |     v    v
DELIVERED FAILED
```

A integração do ACK fim-a-fim ao pacote físico será feita depois da validação da semântica.
