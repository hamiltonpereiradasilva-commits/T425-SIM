# V2.65 — resultado da auditoria

Arquivos auditados:

- `CMakeLists.txt` raiz;
- `firmware_pico/CMakeLists.txt`;
- `pico_sdk_import.cmake`;
- configuração de `t425_node_config`.

Foi encontrado um conflito arquitetural real: havia um target Pico com `T425_NODE_ID`
como definição de compilação.

A v2.65 remove essa definição do target.

O projeto ainda possui outros artefatos históricos de build que devem ser
tratados como variantes/testes e não como fonte de verdade do firmware final.

A partir daqui, o CMake oficial do diretório `firmware_pico` deve ser a referência
para o firmware persistente.
