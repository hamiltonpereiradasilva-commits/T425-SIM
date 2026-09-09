# V2.75 — primeiro boot com Flash virgem

A Flash pode estar inteiramente apagada após a gravação do UF2.

Portanto:

```text
A = inválido
B = inválido
```

é um estado normal de fábrica.

No primeiro `COMMIT`:

```text
active = NONE
target = A
generation = 1
```

O slot A é gravado e passa a ser a configuração ativa.

No segundo `COMMIT`:

```text
active = A
target = B
generation = 2
```

e assim sucessivamente, alternando A/B.
