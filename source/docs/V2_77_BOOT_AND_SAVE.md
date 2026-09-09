# V2.77 — comportamento esperado

## Unidade virgem

```text
A = invalid
B = invalid

LOAD  -> FAIL / DEFAULT
SAVE  -> A:generation=1
LOAD  -> configuração de A
```

## Segundo SAVE

```text
A:generation=1
B=invalid

SAVE -> B:generation=2
```

## Terceiro SAVE

```text
A:generation=1
B:generation=2

SAVE -> A:generation=3
```

A regra é sempre escrever o slot inativo.

## CLEAR

`CLEAR` executa erase dos 8192 bytes reservados. Depois:

```text
A = invalid
B = invalid
```
