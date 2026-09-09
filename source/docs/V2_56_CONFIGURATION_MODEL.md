# Modelo de configuração v2.56

A identidade do nó é um parâmetro:

```text
NODE_ID = N
```

Os enlaces são dados:

```text
IF0 -> PEER_ID
IF1 -> PEER_ID
...
```

Exemplo:

```text
P00:
  IF0 -> P01
  IF1 -> P02

P03:
  IF0 -> P01
  IF1 -> P02
```

O mesmo executável pode ser usado nos dois.

A inicialização transforma o perfil em uma tabela operacional.

```text
PROFILE
   |
   v
NODE_CONFIG
   |
   +--> NODE_ID
   +--> INTERFACE_TABLE
```
