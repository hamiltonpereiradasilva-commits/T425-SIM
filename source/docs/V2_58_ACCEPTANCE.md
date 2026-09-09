# V2.58 — critérios de aceitação

1. `GET_CONFIG` deve ser reconhecido.
2. `SET_NODE_ID` deve aceitar 0..255.
3. `SET_INTERFACE` deve validar índice e parâmetros.
4. `VALIDATE` deve ser uma operação explícita.
5. `COMMIT` não deve gravar diretamente sem passar pela transação.
6. Comandos inválidos devem retornar erro.
7. O protocolo USB deve permanecer independente do Router.
8. O mesmo firmware deve aceitar NODE_ID diferente em cada Pico.
