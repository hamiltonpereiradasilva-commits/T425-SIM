# T425-SIM v2.21 — USB application console

A v2.21 cria a primeira interface de aplicação para o MacBook.

## Comandos

```text
HELP
STATUS
ROUTES
STATS
PING 1
SEND 3 HELLO
```

### `SEND`

```text
SEND <DEST> <TEXT>
```

Exemplo:

```text
SEND 3 HELLO
```

A aplicação cria um pacote:

```text
SOURCE = NODE_ID local
DESTINATION = 3
TTL = 8
SEQ = contador local
PAYLOAD = HELLO
```

e o serializa através do Transport.

## Observação

Nesta versão a console já constrói e valida o pacote, mas a saída ainda é o ponto de integração para a camada física. O próximo passo é ligar `SEND` ao Router + Reliable Link + PIO, sem duplicar a lógica de serialização.
