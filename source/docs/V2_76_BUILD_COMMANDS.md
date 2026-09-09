# v2.76 — build gate

Ambiente necessário:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S firmware_pico -B build -DPICO_BOARD=pico
cmake --build build -j
```

Após o build, auditar o `.map`:

```text
FLASH origin  = 0x10000000
FLASH length  = 0x1FE000
config start  = 0x101FE000
config end    = 0x10200000
```

Esses valores assumem Pico com 2 MiB.

O pacote não declara um UF2 gerado nesta sessão sem um Pico SDK/toolchain
realmente instalado.
