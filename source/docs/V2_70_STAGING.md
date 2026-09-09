# T425-SIM v2.70 — staging real de configuração

O `SET_CONFIG` agora tem semântica efetiva:

```text
USB frame
   ↓
parse header
   ↓
validate structure
   ↓
copy CONFIG_RECORD
   ↓
RAM staging
```

Nenhum erase/program ocorre durante `SET_CONFIG`.

`GET_CONFIG` devolve o conteúdo atualmente staged na sessão.

## Isolamento

```text
USB parser
    ↓
USB session
    ↓
CONFIG staging
```

A Flash continua atrás do transaction manager.
