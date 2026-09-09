# V2.69 — sessão host/Pico

Sessão prevista:

```text
HOST -> PING
PICO -> OK

HOST -> GET_CONFIG
PICO -> CONFIG

HOST -> SET_CONFIG
PICO -> STAGED

HOST -> COMMIT
PICO -> COMMITTED / ERROR

HOST -> REBOOT
PICO -> OK
```

O `COMMIT` deve ser idempotente do ponto de vista do estado persistente:
falhas não podem destruir a configuração válida anterior.
