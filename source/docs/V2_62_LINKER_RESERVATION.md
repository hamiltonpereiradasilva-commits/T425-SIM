# T425-SIM v2.62 — reserva da região persistente

A v2.62 transforma a região de configuração em uma responsabilidade explícita
do linker.

A área reservada é:

```text
8192 bytes = SLOT A (4096) + SLOT B (4096)
```

Ela deve ficar fora da região usada para a imagem executável.

O endereço inicial não é fixado por número mágico. Ele é derivado do final da
região de Flash disponibilizada ao aplicativo:

```text
CONFIG_START = FLASH_END - 8192
CONFIG_END   = FLASH_END
```

O projeto deve ter uma única fonte de verdade para `FLASH_ORIGIN` e
`FLASH_LENGTH`.

## Regra

Não manter um segundo linker script independente somente para configuração.
A reserva deve ser integrada ao linker script oficial do firmware.
