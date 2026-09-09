# Testes v1.7

1. Criar dois processos.
2. Criar canal.
3. `IN` sem mensagem → processo bloqueia.
4. `OUT` → mensagem entra e consumidor desbloqueia.
5. `IN` → mensagem recebida.
6. Scheduler escolhe maior prioridade READY.

Resultado esperado: PASS.
