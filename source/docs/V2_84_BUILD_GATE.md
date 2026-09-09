# T425-SIM v2.84 — primeiro build real

Esta revisão não declara um build que não foi executado.

Foi acrescentado `build_gate.sh`, que:

1. verifica `cmake`;
2. verifica `arm-none-eabi-gcc`;
3. verifica `PICO_SDK_PATH`;
4. executa `cmake -S . -B build -DPICO_BOARD=pico`;
5. executa `cmake --build build --parallel`;
6. procura ELF/MAP/BIN/UF2;
7. extrai do `.map` `__t425_config_start__` e `__t425_config_end__`.

No ambiente desta sessão, a etapa ARM continua condicionada à presença real do
SDK/toolchain.

O Pico SDK implementa `pico_override_flash_size()` configurando
`pico_flash_region.ld` e registrando-o como override do target. citeturn0search0

A própria documentação/código de exemplo do SDK usa `flash_safe_execute()`
ao redor de erase/program porque o código normalmente executa da Flash via
XIP. citeturn0search1turn0search3
