# v2.86 — próximo passo

Para executar o primeiro build real:

1. obter uma cópia compatível do Raspberry Pi Pico SDK;
2. colocar `external/pico_sdk_import.cmake` no projeto;
3. disponibilizar `PICO_SDK_PATH`;
4. disponibilizar `arm-none-eabi-gcc`;
5. executar `./build_gate.sh`.

O próprio Raspberry Pi recomenda CMake + GCC cross compiler e `PICO_SDK_PATH`
para o fluxo C/C++ do Pico. citeturn0search1turn0search24

Somente após isso serão analisados:
- erros de compilação;
- erros de link;
- tamanho do firmware;
- `.map`;
- ELF;
- BIN;
- UF2.
