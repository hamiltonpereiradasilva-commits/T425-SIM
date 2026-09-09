# v2.84 — auditoria aritmética do mapa

Para o alvo `pico`:

- Flash física: `2,097,152` bytes = `0x200000`
- região persistente: `8,192` bytes = `0x2000`
- Flash de firmware: `2,088,960` bytes = `0x1FE000`
- XIP base: `0x10000000`
- início persistência: `0x10000000 + 0x1FE000 = 0x101FE000`
- fim: `0x10200000`

Isso é uma verificação aritmética independente do linker. O `.map` continua
sendo necessário para provar que o linker aplicou exatamente esse contrato.
