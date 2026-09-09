# T425-SIM v2.58 — console USB de configuração

A v2.58 cria a camada de comandos que será transportada pelo USB CDC do Pico.

Comandos:

```text
GET_CONFIG
SET_NODE_ID <id>
SET_INTERFACE <if> <peer> <pio_sm> <tx_gpio> <rx_gpio>
VALIDATE
COMMIT
```

Exemplo:

```text
SET_NODE_ID 4
SET_INTERFACE 0 1 0 2 3
SET_INTERFACE 1 2 1 4 5
VALIDATE
COMMIT
```

O parser altera somente a configuração em RAM.

`COMMIT` apenas solicita a gravação; a gravação transacional em Flash continua
sendo uma etapa separada.

## Razão dessa separação

Não devemos permitir que um comando recebido pela USB altere imediatamente a
configuração ativa de hardware sem validação.

Fluxo correto:

```text
USB
 |
parser
 |
RAM CONFIG
 |
VALIDATE
 |
COMMIT REQUEST
 |
Flash transaction
 |
verify
 |
activate
 |
reboot
```
