# T425-SIM RP2040 — v0.6 VALIDADA

A v0.6 introduz a camada OCCAM-like e seleção ALT canal/timer.

## Componentes
- IR experimental: `SEQ`, `IN`, `OUT`, `ALT`, `TIMER AFTER`, `END`;
- validador de IR;
- descritor de processo/runtime;
- scheduler e canais;
- timer monotônico;
- guards temporais;
- seleção ALT;
- exemplo `occam/sender_receiver.occ`;
- testes host-side.

## Correção de integração
O tipo de guarda temporal foi renomeado para `t425_timer_guard_t`, evitando colisão com o tipo de guarda composto do módulo ALT.

## Limitação
A IR não é o compilador OCCAM histórico e não representa ainda a ISA binária completa do T425. O objetivo é validar a semântica concorrente antes de gerar código T425.
