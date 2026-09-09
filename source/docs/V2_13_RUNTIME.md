# T425-SIM v2.13 — Node Runtime

A v2.13 integra as camadas de transporte em uma máquina de execução de nó.

```text
                 T425 NODE RUNTIME
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
       RX SERVICE     ACK/TIMER      TX SERVICE
          │              │              │
       Stream           Retry         Scheduler
       Parser            │              │
          │              │              │
          └──────────────┼──────────────┘
                         ▼
                    Node Manager
```

O runtime mantém um parser e um controlador ACK por link habilitado.

## Separação

A camada genérica contém:
- estado do nó;
- configuração;
- parser;
- ACK/retry;
- contador de ticks.

A camada RP2040/HAL deverá fornecer:
- relógio/timer;
- PIO;
- GPIO;
- USB CDC;
- armazenamento da configuração.

Essa separação é necessária para preservar a portabilidade do núcleo T425.
