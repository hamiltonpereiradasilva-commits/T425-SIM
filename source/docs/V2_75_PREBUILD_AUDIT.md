# T425-SIM v2.75 — auditoria pré-build

A revisão da v2.74 encontrou defeitos de integração que impediam considerar o
projeto build-ready.

## Correções

1. **CMake**: os novos módulos USB/Flash não estavam efetivamente na lista do
   `add_executable()`.
2. **Validação canônica**: havia referência a `record->node_id`, embora
   `node_id` esteja dentro de `record.config`. A validação agora usa
   `t425_config_validate()`, que é a fonte canônica.
3. **Slot vazio**: leitura física de slot inválido/virgem não é erro de I/O.
   `boot` precisa conseguir ler A/B mesmo quando ambos estão em `0xFF`.
4. **Primeiro COMMIT**: quando A e B são inválidos, o transaction manager já
   pode preparar A com generation=1; o caminho de boot agora não bloqueia essa
   situação.
5. **CMake/linker**: o override passa a substituir exatamente
   `pico_flash_region.ld`, como exige o mecanismo do Pico SDK.
6. **Símbolos**: `__t425_config_start__` e `__t425_config_end__` são exportados
   pelo linker via `pico_set_linker_script_var()`.
7. **Hardware facade**: incluído explicitamente `t425_flash_transaction.h`
   para eliminar dependência de declaração implícita.

## Situação

Esta versão é a primeira que deve ser tratada como **pré-build coerente**.
Ainda não é declarada como "build valid" sem executar CMake + ARM GCC + Pico
SDK.
