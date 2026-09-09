# T425-SIM v2.79 — primeiro build gate

## Resultado desta revisão

Foi executada uma auditoria estrutural adicional da árvore v2.78.

Foi encontrado e corrigido um erro de contrato em `t425_config_staging.c`:
o código consultava `staging.record.node_id`, embora `node_id` pertença a
`staging.record.config.node_id`. A validação agora usa
`t425_config_validate(&s->record)`, eliminando também uma validação duplicada.

## Build real

O ambiente atual possui `cmake`, porém não possui:

```text
PICO_SDK_PATH
arm-none-eabi-gcc
```

Consequentemente, **não foi possível executar honestamente o primeiro build
RP2040 nesta sessão**.

Não há `.elf`, `.map` ou `.uf2` alegados como válidos nesta versão.

O SDK oficial confirma o fluxo:

```text
cmake -S . -B build
cmake --build build
```

e `pico_add_extra_outputs()` produz ELF/MAP/BIN/UF2. 

A função `pico_override_flash_size()` também é oficial do SDK e gera o
`pico_flash_region.ld` usado como override.

## Comando recomendado

```bash
export PICO_SDK_PATH=/caminho/para/pico-sdk
cmake -S firmware_pico -B build -DPICO_BOARD=pico
cmake --build build -j
```

Depois:

```bash
ls -lh build/t425_pico_v15.*
```

## Auditoria obrigatória do MAP

Confirmar:

```text
__t425_config_start__ = 0x101FE000
__t425_config_end__   = 0x10200000
```

e que nenhum `.text`, `.rodata`, `.data` ou outra seção do firmware ocupa
a faixa `[0x101FE000, 0x10200000)`.

## Observação

`flash_range_erase()` recebe offset relativo à Flash e exige alinhamento de
setor; `flash_range_program()` exige alinhamento de página e quantidade
múltipla de 256 bytes. `flash_safe_execute()` é o mecanismo oficial para
entrar na região segura de erase/program.
