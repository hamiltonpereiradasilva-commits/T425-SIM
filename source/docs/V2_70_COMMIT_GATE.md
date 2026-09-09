# V2.70 — gate do COMMIT

O `COMMIT` continua bloqueado nesta versão até que o registro canônico seja
conectado ao `t425_flash_transaction()`.

Isso evita um erro perigoso: permitir que o protocolo USB grave um registro
que foi apenas estruturalmente parseado, sem passar pela validação completa e
pela política A/B.

Sequência obrigatória futura:

```text
SET
 ↓
staging
 ↓
canonical validation
 ↓
current A/B
 ↓
inactive target
 ↓
erase/program/read-back
 ↓
CRC
 ↓
commit
```
