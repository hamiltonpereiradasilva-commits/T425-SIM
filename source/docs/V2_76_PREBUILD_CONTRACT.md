# v2.76 — pre-build contract audit

Correções desta etapa:

1. `flash_range_erase/program` recebem **Flash offsets**, não XIP addresses.
   O SDK define esses argumentos como offsets relativos ao início da Flash.
2. O tamanho de linkedição é reduzido por `pico_override_flash_size()`, que
   é a abstração fornecida pelo Pico SDK para gerar/override de `pico_flash_region.ld`.
3. Os símbolos `__t425_config_start__` e `__t425_config_end__` ficam em um
   fragmento separado, evitando transformar o fragmento de região de memória
   no linker script principal.
4. Foram adicionados `_Static_assert` para A/B, setor e página.

O SDK confirma que `flash_range_erase()` recebe offsets alinhados a 4096 e que
`flash_range_program()` recebe offsets alinhados a 256 e contagens múltiplas
de 256. citeturn0search0turn0search4
