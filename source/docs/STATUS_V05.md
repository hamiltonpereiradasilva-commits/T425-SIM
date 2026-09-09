# T425-SIM RP2040 — v0.5 — RELEASE CANDIDATE

A camada de runtime foi corrigida para manter sincronizados os estados do descritor do runtime e do descritor arquitetural da VM durante bloqueio/desbloqueio de canais.

## Componentes
- VM T425;
- scheduler;
- descritores de processo com PC/W/SP/prioridade/workspace;
- START/STOP/RUN/END no runtime;
- canais rendezvous;
- timer monotônico e guards temporais;
- infraestrutura para ALT;
- testes host-side.

## Critério de validade
Os testes host-side devem compilar e terminar com código 0. Isso valida a camada de software portátil; ainda não substitui síntese/execução no RP2040.

## Próximo marco
v0.6: workspace real integrado à VM, ALT canal+timer, primitivas de processo em nível de instrução, e primeiro programa OCCAM distribuído.
