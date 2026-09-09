# Topologias suportadas pelo modelo

## Linear

```text
P00 ─ P01 ─ P02 ─ P03 ─ P04
```

## Árvore

```text
          P00
        /    \
      P01    P02
             /  \
           P03  P04
```

## Malha parcial

```text
P00 ─── P01
 │       │
 │       │
 P02 ─── P03
```

A implementação atual fornece até quatro links físicos por nó. A tabela de vizinhança é que determina quais links estão efetivamente ativos.
