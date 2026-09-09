# v2.86 — blocker encontrado antes do ARM build

A execução real da v2.85 mostrou ausência de `arm-none-eabi-gcc`. A inspeção
da árvore também mostrou que o projeto precisa declarar explicitamente a
importação do Pico SDK antes de `project()`.

A documentação oficial do Pico SDK determina:

```cmake
include(pico_sdk_import.cmake)
project(...)
pico_sdk_init()
```

e orienta que `pico_sdk_import.cmake` seja copiado de
`pico-sdk/external/` ou que `PICO_SDK_PATH` seja fornecido. citeturn0search1

A v2.86 adiciona essa dependência explicitamente e faz o gate falhar com uma
mensagem específica se o arquivo não estiver presente.

Também não foi fabricado um `pico_sdk_import.cmake`: ele deve vir do SDK real,
evitando divergência silenciosa entre versões.

O SDK oficial confirma ainda que `pico_override_flash_size(TARGET FLASH_SIZE)`
configura `pico_flash_region.ld` e registra o caminho como override do linker.
citeturn0search0
