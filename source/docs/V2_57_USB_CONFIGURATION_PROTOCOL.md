# V2.57 — protocolo USB de configuração

A configuração ainda não é gravada pela aplicação USB nesta versão; esta
etapa define o formato persistente e sua validação.

A próxima camada deverá expor comandos de configuração pelo USB:

```text
GET_CONFIG
SET_NODE_ID
SET_INTERFACE
COMMIT
RESET_CONFIG
```

Fluxo esperado:

```text
MacBook
  |
 USB CDC
  |
 Config Console
  |
 validate
  |
 write inactive Flash slot
  |
 verify
  |
 activate
  |
 reboot
```

A gravação deve ser transacional: primeiro escrever uma cópia nova e validá-la;
somente depois torná-la ativa.

Não se deve apagar a única configuração válida antes de existir uma nova cópia
válida.
