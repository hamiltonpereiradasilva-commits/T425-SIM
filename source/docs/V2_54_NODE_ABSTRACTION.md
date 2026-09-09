# V2.54 — abstração do nó

Todo Pico executa a mesma arquitetura de firmware:

```text
NODE
 |
 +-- Interface Manager
 |    +-- IF0
 |    +-- IF1
 |    +-- ...
 |
 +-- Link Layer
 |
 +-- Router
 |
 +-- E2E
 |
 +-- Application
 |
 +-- USB Console
```

A identidade do nó não deve ser codificada em lógica de protocolo.

Ela deve vir de configuração:

```text
NODE_ID
INTERFACE_TABLE
```

Assim o mesmo binário pode ser reutilizado nos quatro Pico, desde que a
configuração de inicialização determine a identidade e o mapeamento físico.
