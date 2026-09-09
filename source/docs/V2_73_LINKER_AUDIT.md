# T425-SIM v2.73 — linker e mapa de memória

A região persistente passa a ser derivada do fim da região `FLASH` do linker:

```text
FLASH
+-----------------------------+
| firmware / rodata / etc.    |
|                             |
| ...                         |
+-----------------------------+
| T425 CONFIG REGION          |
| 8192 bytes                  |
|                             |
| slot A | slot B             |
+-----------------------------+
^                             ^
__t425_config_start__          __t425_config_end__
```

A região não usa endereço absoluto no código C.

O linker verifica alinhamento em setor de 4096 bytes e tamanho de 8192 bytes.

## Correção importante da v2.72

`flash_range_program()` exige endereço alinhado a 256 bytes e `count` múltiplo
de 256. A v2.72 não poderia programar `sizeof(record)` diretamente se esse
tamanho não fosse múltiplo de página.

A v2.73 corrige isso programando o slot inteiro, preenchendo o restante com
`0xFF`.

A documentação oficial do Pico SDK confirma essas restrições. 
