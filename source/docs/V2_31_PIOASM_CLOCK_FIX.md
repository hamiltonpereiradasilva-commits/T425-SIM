# T425-SIM v2.31 — PIOASM + clock correction

A v2.31 corrige dois pontos da v2.30:

1. os `.pio.h` não são mais tratados como arquivos manuais;
2. a divisão do clock PIO usa `clock_get_hz(clk_sys)`.

O Pico SDK documenta que `pioasm` gera os headers C e que `pico_generate_pio_header(TARGET FILE)` integra essa geração ao CMake. Isso evita divergência entre o programa `.pio` e seu header gerado.

A implementação também consulta o clock de sistema real do RP2040 no momento da inicialização da PHY.

## Estado

- Fonte PIO: preparada.
- Geração automática dos headers: preparada.
- Clock real: integrado.
- Hardware P00↔P01: ainda não medido.

A próxima etapa deve ser um firmware de bancada mínimo, sem Router/OCCAM, que produza e receba um padrão conhecido e permita verificar o enlace com analisador lógico.
