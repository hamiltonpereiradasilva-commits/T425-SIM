# T425-SIM v2.78 — auditoria pré-build corrigida

A revisão estrutural da v2.77 encontrou três defeitos concretos.

## A. API de validação

` t425_flash_slots.h ` exporta `t425_flash_slot_valid()`, mas o driver físico
usava o nome inexistente `t425_flash_slot_validate()`.

Corrigido.

## B. Endereço XIP versus offset Flash

Os símbolos `__t425_config_start__` e `__t425_config_end__` são endereços do
espaço XIP. Para leitura, o ponteiro XIP é usado diretamente.

Para erase/program, o driver calcula:

```text
flash_offset = xip_address - XIP_BASE
```

Isso corresponde ao contrato de `flash_range_erase()` e
`flash_range_program()`, que recebem offsets relativos ao início da Flash.
A documentação do SDK também exige erase em múltiplos de 4096 e program em
múltiplos de 256 bytes. citeturn0search1turn0search4

## C. Linker

A v2.78 abandona a reprodução manual do mecanismo interno do SDK e usa
`pico_override_flash_size()` — a API oficial do `pico_standard_link` — para
gerar o `pico_flash_region.ld`. Em seguida, os símbolos T425 são anexados ao
arquivo efetivamente utilizado como override. citeturn0search0

## D. Geração

Foi adicionada proteção contra overflow de `generation` em `UINT32_MAX`.
Não é permitido gerar silenciosamente `generation=0`.

## E. Contratos

O arquivo `t425_static_contracts.c` passa a fazer parte do target e transforma
os requisitos A/B em `_Static_assert`.

## Resultado

A v2.78 é uma base pré-build mais consistente. Ainda não é declarado um UF2
validado porque a compilação real requer o Pico SDK + toolchain ARM.
