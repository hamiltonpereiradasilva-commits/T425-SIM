# V2.78 — gate do primeiro build

O próximo ambiente deve possuir:

- PICO_SDK_PATH;
- ARM GCC;
- CMake;
- suporte UF2 do Pico SDK.

Comandos esperados:

```text
cmake -S firmware_pico -B build -DPICO_BOARD=pico
cmake --build build -j
```

Depois auditar:

```text
build/t425_pico_v15.elf
build/t425_pico_v15.map
build/t425_pico_v15.uf2
```

O `.map` deve mostrar firmware limitado a 2 MiB - 8192 e os símbolos:

```text
__t425_config_start__ = 0x101FE000
__t425_config_end__   = 0x10200000
```

para um Pico RP2040 padrão de 2 MiB.
