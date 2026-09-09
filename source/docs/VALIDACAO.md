# Plano de validação P00–P03

## Teste 1 — identidade

Cada nó deve imprimir:

`T425-SIM RP2040 NODE N`

com N = 0,1,2,3.

## Teste 2 — RAM

Escrever e ler padrões em:

- `0x80000000`
- `0x80000070`
- `0x80000FFC`

Não acessar fora de `0x80000000–0x80000FFF` nesta fase.

## Teste 3 — ISA mínima

Programas de teste devem validar:

- LDC
- LDL
- LDLP
- LDNL
- LDNL P
- ADC
- STL
- STNL
- AJW
- EQC
- J
- OPR
- ADD/SUB
- AND/OR/XOR/NOT
- SHL/SHR
- DUP/POP

## Teste 4 — canais

Validar produtor/consumidor no mesmo nó antes de distribuir processos.

## Teste 5 — links

P00→P01, P01→P02, P02→P03 e P03→P00.

Depois validar uma topologia mesh/ring.

## Teste 6 — controle

O MacBook/ESP32 deverá executar:

PING → RESET → LOAD → READ_REGS → RUN → HALT → STEP.

## Critério

A implementação somente deve avançar para a próxima camada depois que a
camada anterior possuir testes reproduzíveis.
