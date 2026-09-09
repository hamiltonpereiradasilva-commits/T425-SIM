# T425-SIM v2.63 — binding físico da Flash

A v2.63 cria a última fronteira antes da operação física.

A regra é rígida:

```text
ACTIVE SLOT
    |
    X  nunca apagar
    |
INACTIVE SLOT
    |
    +-- erase
    +-- program
    +-- read-back
    +-- CRC validation
    |
    v
NEW ACTIVE
```

A rotina `t425_flash_real_write_slot()` não executa erase/program enquanto a
versão exata do Pico SDK e o linker script oficial não estiverem vinculados ao
projeto.

Isso é intencional: uma chamada de Flash mal endereçada pode destruir a imagem
do firmware.
