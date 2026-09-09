# Compilação

## Dependências

- Raspberry Pi Pico SDK
- CMake
- ARM GNU Toolchain

Defina:

```bash
export PICO_SDK_PATH=/caminho/para/pico-sdk
```

Copie o `pico_sdk_import.cmake` oficial do SDK para a raiz do projeto.

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

Serão produzidos:

```text
build/t425_pico0.uf2
build/t425_pico1.uf2
build/t425_pico2.uf2
build/t425_pico3.uf2
```

Cada UF2 é funcionalmente igual, exceto pelo `T425_NODE_ID`.

## Gravação

Coloque o Pico em BOOTSEL e copie o UF2 correspondente.

A ordem sugerida para o laboratório é:

- Pico 0 → P00
- Pico 1 → P01
- Pico 2 → P02
- Pico 3 → P03
