# T425-SIM RP2040 — v0.4

## Objetivo
Integrar VM, scheduler e canais em um runtime único para execução concorrente.

## Implementado
- `STARTP`/criação de processo via API runtime;
- `RUNP`, `STOPP`, `ENDP` via API runtime;
- processo com PC, W e prioridade;
- bloqueio automático em `IN`/`OUT` sem parceiro;
- desbloqueio do parceiro quando ocorre rendezvous;
- scheduler integrado ao runtime;
- teste host-side de processo + scheduler + canal.

## Importante
A API runtime reproduz o modelo conceitual das operações de processo/canal. A codificação binária completa das instruções OPR e a organização exata dos descritores de workspace do T425 ainda precisam ser fechadas antes de afirmar compatibilidade binária com toolchains INMOS.
