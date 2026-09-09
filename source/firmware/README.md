# Firmware RP2040 v2.3

Firmware orientado ao Pico SDK.

## Compilação

Defina `PICO_SDK_PATH` e use o fluxo padrão CMake/Ninja do Pico SDK.

Exemplo:

```text
mkdir build
cd build
cmake ..
ninja
```

O resultado esperado é `t425_node.uf2`.

## Importante

O código desta versão é um *bring-up skeleton*: inicializa USB CDC e anuncia READY. O driver PIO de produção será integrado depois da validação elétrica/temporal do enlace.
