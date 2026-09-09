# T425-SIM v1.3 — Firmware RP2040/Pico SDK

## Objetivo

Criar o primeiro alvo de firmware real para RP2040 usando Pico SDK, PIO, GPIO e USB stdio.

### Pinagem de laboratório proposta

- Link 0: TX GP4, RX GP5
- Link 1: TX GP6, RX GP7

A pinagem deve ser conferida na revisão física do Pico/carrier antes da conexão.

### Build

Definir `PICO_SDK_PATH` no ambiente e executar CMake no diretório `firmware_pico`.

O alvo gera ELF/UF2 através de `pico_add_extra_outputs()`.

### Observação de engenharia

A implementação usa duas state machines por link, mas o divisor de clock/baud e o programa PIO ainda precisam de calibração no osciloscópio/analisador lógico. O firmware é o primeiro bring-up, não uma declaração de conformidade elétrica.
