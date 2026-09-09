# Exemplos de configuração

## P00

```text
NODE_ID=0
LINK0 -> neighbor 1
```

## P01

```text
NODE_ID=1
LINK0 -> neighbor 0
LINK1 -> neighbor 2
```

## P02

```text
NODE_ID=2
LINK0 -> neighbor 1
LINK1 -> neighbor 3
```

## P03

```text
NODE_ID=3
LINK0 -> neighbor 2
```

Uma futura bancada pode usar:

```text
P00-P01-P02-P03-P04-P05-P06-P07
```

sem alterar o modelo de firmware: somente a configuração dos nós muda.
