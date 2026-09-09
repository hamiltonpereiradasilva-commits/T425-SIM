# V2.74 — gate de build

A v2.74 só pode ser declarada "build-valid" quando executada em ambiente que
tenha:

- Pico SDK;
- ARM GCC;
- CMake;
- ferramenta UF2/picotool conforme o fluxo do projeto.

O artefato desta versão contém a integração correta, mas não afirma que um
`.uf2` foi gerado nesta sessão se o SDK/toolchain não estiver disponível.

Após o build, auditar:

```text
Memory region FLASH
__t425_config_start__
__t425_config_end__
```

e o `.map`.
