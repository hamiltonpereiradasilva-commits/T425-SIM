# V2.69 — revisão corrigida

A colisão preliminar de códigos foi corrigida antes do congelamento:

```text
PING       = 0x01
GET_CONFIG = 0x02
SET_CONFIG = 0x03
COMMIT     = 0x04
REBOOT     = 0x05
```

Todos os comandos possuem códigos únicos.

A implementação completa de staging/commit permanece para a próxima etapa.
