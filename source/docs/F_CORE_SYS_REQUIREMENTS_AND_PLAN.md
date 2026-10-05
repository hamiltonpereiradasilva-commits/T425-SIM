# F_CORE-SYS-01 - RESET AND BOOTSTRAP

Arquivos alterados:
- source/t425_core/include/t425_memory.h: constantes MemStart e ROM entry.
- source/t425_core/include/t425_core.h: estados e campos System/Bootstrap.
- source/t425_core/src/t425_core.c: inicializacao do estado.
- source/t425_core/src/t425_core_system.c: FSM de Reset, ROM, PEEK, POKE e LOAD.
- source/t425_core/instruction/t425_instruction.c: prioridade da FSM em t425_core_step.
- source/t425_canonical/CMakeLists.txt: unidade CORE System.
- tests/t425_core/test_f_core_sys_01_bootstrap.c: regressao host.

## F_CORE-SYS-01 - RESET AND BOOTSTRAP

t425_core_reset() continua sendo inicializacao da instancia: inicializa CPU,
memoria, scheduler, timers, datapath e microsequencer. O Reset arquitetural
lido pela seam portatil nao chama t425_memory_reset(): reinicializa CPU,
scheduler, timers, datapath e execucao, preservando a memoria. BootFromROM e
amostrado uma vez no release do Reset.

A FSM possui RESET, BOOT_DECIDE, BOOT_WAIT_CONTROL, BOOT_POKE_ADDRESS,
BOOT_POKE_DATA, BOOT_PEEK_ADDRESS, BOOT_PEEK_OUTPUT, BOOT_LOAD e RUNNING.
Cada t425_core_step() executa no maximo um avanco e nunca bloqueia.

- RESET: bloqueia execucao e consumo enquanto Reset esta ativo.
- BOOT_DECIDE: HIGH configura ROM boot; LOW espera controle.
- BOOT_WAIT_CONTROL: escolhe o primeiro Link0..3 que fornece controle.
- BOOT_POKE_ADDRESS/DATA: recebe quatro bytes LSB-first e escreve memoria.
- BOOT_PEEK_ADDRESS/OUTPUT: le memoria e envia quatro bytes LSB-first; TX ocupado preserva byte_index.
- BOOT_LOAD: recebe exatamente count bytes em MemStart.
- RUNNING: entrega ao fluxo normal.

Link0..3 sao IDs explicitos 0..3. t425_channel_select nao participa.
Physical ACK, tx_waiting_ack, PIO e DMA nao aparecem nesta FSM.

ROM boot usa IptrReg = 0x7FFFFFFE, WptrReg = MemStart e WdescReg =
MemStart | T425_PRIORITY_LOW. LOAD usa IptrReg = MemStart e
WptrReg = align_up(MemStart + count, 4), com prioridade LOW.

Callbacks System/PHY sao opcionais; sem PHY anexada, o CORE preserva o fluxo
existente de testes host.

### F_CORE-SYS-01B — Reset context invalidation

Architectural Reset preserves T425 RAM but invalidates every internal emulator `process_contexts[]` and `interrupt_contexts[]` entry, including their saved fields. Instance initialization remains responsible for the full initial reset. Boot Wdesc encoding is retained and checked against the canonical definition: `Wdesc = Wptr | 1`, with LOW priority represented by bit 0.
