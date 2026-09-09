# V2.78 — semântica física da Flash

O SDK define:

```text
flash_range_erase(offset, count)
    offset: alinhado a 4096
    count : múltiplo de 4096

flash_range_program(offset, data, count)
    offset: alinhado a 256
    count : múltiplo de 256
```

A v2.78 programa um slot inteiro de 4096 bytes, preenchendo com `0xFF` o
espaço não ocupado pelo registro.

Para XIP:

```text
slot_xip = __t425_config_start__ + slot * 4096
```

Para programação:

```text
slot_offset = slot_xip - XIP_BASE
```

Para leitura:

```text
ptr = (const uint8_t *)slot_xip
```

Essa distinção é obrigatória e agora está explícita no driver.
