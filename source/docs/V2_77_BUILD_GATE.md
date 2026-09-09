# V2.77 — gate de build

Esta versão é **pré-build**, não um UF2 validado.

O ambiente desta sessão não possui `PICO_SDK_PATH`, portanto não é possível
afirmar compilação/linkagem real.

O primeiro build externo deve produzir:

- ELF;
- MAP;
- BIN;
- UF2.

A auditoria deve confirmar:

```text
FLASH link-time size = 2 MiB - 8192
config_start = 0x101FE000
config_end   = 0x10200000
```

para o Pico RP2040 padrão de 2 MiB.

Também deve confirmar ausência de referências a:
- `pico_set_linker_script_var`;
- `t425_config_symbols.ld` como override independente;
- endereços absolutos antigos de Flash.
