# v2.87 — toolchain gate

O projeto agora exige o SDK real e o toolchain real. `pico_sdk_import.cmake`
não é recriado pelo projeto; deve vir de `<PICO_SDK_PATH>/external/`.

O fluxo oficial coloca `include(pico_sdk_import.cmake)` antes de `project()` e
usa `PICO_SDK_PATH` para localizar o SDK. O SDK oficial também documenta
`arm-none-eabi-gcc` como toolchain Arm suportado. citeturn0search0turn0search24

A página oficial de releases consultada nesta revisão marca o SDK 2.3.0 como
Latest. citeturn0search2

Esta revisão não declara ELF/MAP/UF2 sem que o build seja executado.
