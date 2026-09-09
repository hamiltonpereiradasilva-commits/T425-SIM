# V2.61 — contrato do driver RP2040

O driver deve oferecer:

```text
init()
read(slot)
erase(slot)
program(slot)
verify(slot)
```

A implementação física deverá usar as primitivas adequadas do SDK do RP2040,
mas somente depois de o linker reservar a região.

A leitura pode ocorrer através do espaço XIP; escrita/erase exige a sequência
de programação de Flash apropriada e cuidados de execução fora da Flash quando
necessário pelo SDK.

Esta versão não executa operações destrutivas.
