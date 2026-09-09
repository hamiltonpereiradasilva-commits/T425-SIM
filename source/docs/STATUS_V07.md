# T425-SIM RP2040 — v0.7 — VALIDADA

A v0.7 adiciona uma máquina executável para a IR OCCAM-like e um simulador de dois processos compartilhando canais.

## Entregue
- interpretador de IR por processo;
- PID lógico estável por processo;
- estados READY/WAIT_IN/WAIT_OUT/WAIT_TIMER/DONE/ERROR;
- variáveis locais;
- IN/OUT bloqueantes;
- modelo PAR de dois processos;
- rendezvous;
- timer;
- parser textual mínimo;
- teste sender/receiver integrado.

## Critério
Todos os testes host-side da release retornaram código 0.
