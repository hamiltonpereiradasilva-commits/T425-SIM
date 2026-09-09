# T425-SIM v2.71 — validação canônica e COMMIT

O ciclo USB agora atravessa a fronteira transacional:

```text
SET_CONFIG
    ↓
RAM staging
    ↓
canonical validation
    ↓
read A/B
    ↓
select active
    ↓
select inactive
    ↓
erase/program/verify via HW
    ↓
new active generation
    ↓
OK
```

A implementação de COMMIT não acessa a Flash diretamente; usa
`t425_flash_transaction()`.

## Regra de falha

Qualquer falha retorna erro e não há uma instrução para apagar o slot ativo.
