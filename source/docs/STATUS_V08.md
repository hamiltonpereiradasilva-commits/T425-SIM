# T425-SIM RP2040 — v0.8 VALIDADA

A v0.8 estabelece o primeiro fluxo estruturado OCCAM → AST → análise semântica → T425-IR.

## Entregue
- AST;
- parser textual;
- `CHAN OF INT`;
- `SEQ` e `PAR` estruturais;
- `IN` e `OUT`;
- `TIMER_AFTER`;
- `END`;
- análise semântica básica;
- compilador AST → IR;
- `occamc`;
- exemplo `p00_p01_demo.occ`;
- regressão completa das funcionalidades v0.7.

## Validação
Todos os testes foram compilados com:
`gcc -std=c11 -Wall -Wextra -Werror`

e executados com retorno 0.

## Limitações explícitas
O `PAR` ainda é estruturalmente linearizado na IR atual; portanto, a compilação de paralelismo real continua sendo responsabilidade do runtime até a v0.9. `ALT` ainda não foi incorporado ao parser/compilador textual completo.
