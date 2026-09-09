# V2.67 — fronteira de runtime

A configuração persistente é uma entrada de runtime.

Ela não deve ser misturada ao processo de compilação:

```text
UF2
 |
 +-- código comum
 |
 +-- configuração local em Flash
          |
          +-- NODE_ID
          +-- parâmetros
          +-- demais campos
```

Assim, a mesma imagem executável continua válida para P00...PN.
