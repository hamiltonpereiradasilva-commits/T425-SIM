# T425-SIM RP2040 — v0.3 corrigida

Esta versão corrige o empacotamento da v0.3 e fecha a primeira camada de execução concorrente.

## Implementado
- VM T425 de 32 bits herdada da v0.2;
- PFIX/NFIX e instruções básicas já presentes na VM;
- canais com rendezvous síncrono;
- estados de processo e scheduler por prioridade;
- salvamento básico de PC/W;
- API isolada para L0..L3;
- estatísticas de links;
- autoteste de ISA no boot;
- teste host-side dos canais;
- documentação e estrutura de testes.

## Limitação deliberada
O codec físico INMOS ainda não é declarado implementado. A camada de link está isolada para que a temporização/handshake INMOS possa ser adicionada sem alterar VM, scheduler ou canais.
