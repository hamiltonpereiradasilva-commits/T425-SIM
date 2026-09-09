# T425-SIM v2.14 — Supervisor e Scheduler

A v2.14 fecha a coordenação do nó.

```text
                SUPERVISOR
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
      RUNTIME             SCHEDULER
          │                   │
     ┌────┼────┐              │
     ▼    ▼    ▼              ▼
    RX   ACK   TX          TIMER/TICK
     │    │    │              │
     └────┴────┴──────────────┘
                    │
                    ▼
                  STATUS
```

O supervisor:
- inicializa runtime;
- inicializa scheduler;
- inicia o nó;
- executa ciclos;
- expõe status textual;
- permite estado de falha.

O scheduler é deliberadamente simples nesta etapa. A política de prioridade, filas e budget de CPU será definida depois que o comportamento físico do PIO for validado.

## Separação HAL

O supervisor não deve acessar GPIO ou registradores diretamente. A futura camada RP2040 HAL será responsável pelo hardware.
