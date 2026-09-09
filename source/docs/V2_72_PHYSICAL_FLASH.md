# T425-SIM v2.72 — Flash física RP2040

A v2.72 substitui o stub do driver físico pelas primitivas do Pico SDK:

```text
flash_range_erase()
flash_range_program()
```

A região não é endereçada por número mágico. O driver usa:

```text
__t425_config_start__
__t425_config_end__
```

fornecidos pelo linker.

## Proteções

Antes de escrever:

- região existe;
- região mede 8192 bytes;
- limites são alinhados a setor;
- slot é 0 ou 1;
- slot cabe integralmente na região;
- registro candidato é válido;
- target é diferente do slot ativo.

Depois da escrita:

```text
Flash → read-back → validação
```

A operação só é aceita se o conteúdo lido de volta for idêntico ao registro
programado e continuar válido.
