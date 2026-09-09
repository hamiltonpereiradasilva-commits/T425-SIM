# Próximo passo

O próximo gate deve ser executado em um ambiente com Pico SDK e ARM GCC.

```text
cmake -S firmware_pico -B build -DPICO_BOARD=pico
cmake --build build -j
```

O resultado deve ser auditado no `.map`, ELF e UF2.

A API oficial `pico_override_flash_size()` gera o `pico_flash_region.ld` e
adiciona o diretório como override do linker. citeturn0search0

Para Flash, o SDK define `FLASH_PAGE_SIZE=256` e `FLASH_SECTOR_SIZE=4096`.
citeturn0search9

Erase/program devem ser executados de modo seguro em relação a XIP, usando
`flash_safe_execute()` quando o código/IRQ/segundo core possa acessar Flash.
citeturn0search1turn0search2
