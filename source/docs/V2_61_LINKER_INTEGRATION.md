# V2.61 — integração com linker

Antes do driver físico final, o projeto precisa ter uma única fonte de verdade
para o tamanho da aplicação e a região persistente.

Objetivo:

```text
| firmware/application | slot A | slot B |
^                      ^         ^
FLASH_BASE             CONFIG    CONFIG
```

A região deve ser excluída da imagem normal da aplicação.

Critério de segurança:

> nenhum byte dos slots de configuração pode pertencer à imagem executável.

Depois de reservado o espaço, o código pode obter ou receber a base dos slots
sem hard-code perigoso.
