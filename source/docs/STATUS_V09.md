# T425-SIM RP2040 — v0.9 VALIDADA FINAL

`PAR` gera imagens de processo independentes. A AST usa `first_child/next_sibling`. O scheduler executa os processos em round-robin e o runner paralelo possui um rendezvous explícito entre instruções `IN`/`OUT`, evitando dupla entrega do mesmo valor.

## Demonstração
P00: OUT(c0,10), OUT(c0,20), END
P01: IN(c0,v0), IN(c0,v1), END

Resultado validado:
v0=10, v1=20.

Todos os testes selecionados são compilados com `-Wall -Wextra -Werror` e retornam 0.
