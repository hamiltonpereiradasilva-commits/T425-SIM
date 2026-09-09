# V2.56 — critérios de aceitação

## A — mesmo código

P00, P01, P02 e P03 devem usar a mesma implementação de:

- Link;
- Interface Manager;
- Router;
- E2E;
- console.

## B — identidade por configuração

Somente `NODE_ID` e a configuração física diferenciam os nós.

## C — sem hard-code de P00/P01/P02/P03 no protocolo

Nomes de nós podem aparecer em perfis/testes, mas a lógica operacional deve usar
IDs e tabelas.

## D — escalabilidade

Adicionar P04 não deve exigir alterar a máquina de estados do protocolo.

Deve ser possível:

```text
carregar o mesmo firmware
+
fornecer NODE_ID=4
+
fornecer suas interfaces
```

## E — limites físicos explícitos

A quantidade máxima de interfaces continua sendo uma limitação da plataforma,
não da semântica do protocolo.
