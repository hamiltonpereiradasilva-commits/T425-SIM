# T425-SIM v1.7 — Runtime OCCAM

A v1.7 implementa o núcleo local de execução:

- processos;
- estados READY/RUNNING/BLOCKED_IN/BLOCKED_OUT/HALTED;
- canais;
- fila de mensagens;
- bloqueio de `IN`;
- bloqueio de `OUT` quando a fila está cheia;
- desbloqueio;
- scheduler por prioridade.

```text
                Scheduler
                    │
       ┌────────────┼────────────┐
       ▼            ▼            ▼
    PROC 0       PROC 1       PROC 2
      │             │
      │ OUT         │ IN
      └──────► CHAN ◄────────────┘
                 │
              queue
```

### Observação semântica

Esta etapa usa uma fila bounded para permitir o desenvolvimento incremental. Ela ainda **não representa o rendezvous estrito OCCAM**, no qual produtor e consumidor sincronizam diretamente. A próxima etapa deve adicionar canais síncronos e canais remotos, preservando o modelo atual como mecanismo de teste.
