# T425-SIM v2.65 — correção da identidade do nó

A auditoria do CMake revelou um ponto que precisava ser corrigido antes de
prosseguir.

O build ainda continha:

```text
T425_NODE_ID=${T425_NODE_ID}
```

Isso contradizia a arquitetura desejada de um único firmware para N nós.

## Correção

O `NODE_ID` foi removido das definições de compilação do target.

Agora:

```text
UF2 único
   |
   +--> P00 -> Flash NODE_ID=0
   +--> P01 -> Flash NODE_ID=1
   +--> P02 -> Flash NODE_ID=2
   +--> P03 -> Flash NODE_ID=3
   +--> P04 -> Flash NODE_ID=4
   +--> ...
```

O firmware passa a carregar a identidade a partir da configuração persistente.

## Resultado

Não é necessário recompilar o firmware para trocar P00 por P07.

A identidade é propriedade da unidade física e não da imagem executável.
