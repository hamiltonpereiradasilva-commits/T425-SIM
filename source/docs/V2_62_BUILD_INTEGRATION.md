# V2.62 — integração de build

A sequência de integração é:

```text
board definition
      |
linker script oficial
      |
FLASH_ORIGIN / FLASH_LENGTH
      |
reserva T425_CONFIG_REGION_SIZE
      |
__t425_config_start__
__t425_config_end__
      |
firmware
```

A compilação deve falhar ou ser rejeitada se:

```text
config_size != 8192
start não alinhado a 4096
```

Também deve ser verificado que o tamanho máximo da imagem executável não invade
a região persistente.

## Observação

O arquivo `t425_config_memory.ld` é um fragmento de contrato, não uma substituição
automática do linker script oficial. A integração final deve respeitar a
organização real do projeto e do SDK utilizado.
