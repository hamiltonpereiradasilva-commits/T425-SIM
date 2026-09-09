# T425-SIM RP2040 — v0.2

## Implementado nesta etapa
- PFIX/NFIX com acumulador de prefixo compatível com a codificação documentada.
- Primárias J, LDLP, PFIX, LDNL, LDC, LDNLP, NFIX, LDL, ADC, CALL, CJ, AJW, EQC, STL, STNL.
- RET e LDPI.
- Operações aritméticas/lógicas e algumas operações auxiliares documentadas.
- Estrutura de scheduler com prioridade e salvamento básico de PC/W.
- Timer-modelo e comando ANALYSE.
- Autoteste de ISA no boot.

## Ainda não é compatibilidade binária/cycle-accurate
Os links físicos ainda são transporte de laboratório; scheduler, canais, bootstrap e timers precisam da implementação completa antes de executar software OCCAM real sem adaptação.
