# V2.58 — transação de configuração

A configuração em RAM é distinta da configuração ativa em Flash.

```text
ACTIVE FLASH CONFIG
        |
        v
    RAM CONFIG
        |
   USB modifications
        |
     VALIDATE
        |
      COMMIT
        |
   FLASH NEW SLOT
        |
      VERIFY
        |
    ACTIVATE
```

Se VALIDATE falhar:

```text
COMMIT = rejected
```

Se a gravação falhar:

```text
ACTIVE CONFIG = unchanged
```

A implementação final deve impedir que uma configuração incompleta se torne
ativa.
