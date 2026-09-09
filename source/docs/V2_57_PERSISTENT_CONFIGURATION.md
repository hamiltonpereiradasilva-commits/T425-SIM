# T425-SIM v2.57 — configuração persistente

A v2.57 substitui os perfis de teste codificados no programa por um modelo de
registro de configuração que pode ser gravado em Flash.

Registro:

```text
+----------------+
| MAGIC          |
+----------------+
| VERSION        |
+----------------+
| LENGTH         |
+----------------+
| NODE_CONFIG    |
|                |
| NODE_ID        |
| INTERFACE TAB  |
+----------------+
| CRC32           |
+----------------+
```

A validação rejeita registros com:

- magic incorreto;
- versão incompatível;
- tamanho incorreto;
- CRC inválido.

## Princípio

O firmware continua comum.

A Flash passa a conter os parâmetros específicos do nó.

```text
FLASH
  |
  +-- NODE_ID
  +-- INTERFACE_TABLE
  +-- CRC
       |
       v
     BOOT
       |
       v
 FIRMWARE COMUM
```

Uma alteração legítima deve gerar um novo CRC. Uma mutação posterior sem
recalcular o CRC deve ser rejeitada.
