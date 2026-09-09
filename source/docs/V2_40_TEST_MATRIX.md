# Matriz de testes v2.40

| Teste | Intervenção | Resultado esperado |
|---|---|---|
| T1 | Sem erro | CRC=0, RETX=0, TIMEOUT=0 |
| T2 | Corrupção automática | CRC>0 |
| T3 | Corrupção + ACK ausente | RETX>0 |
| T4 | Cinco retransmissões | TIMEOUT>0 |
| T5 | Retomar comunicação | novos frames continuam |
| T6 | Ruído/byte inválido | SYNCLOSS/FORMAT podem crescer |

## Ligações

P00 GP2 TX -> P01 GP3 RX
P00 GP3 RX <- P01 GP2 TX
GND -> GND

Cada Pico permanece conectado ao MacBook por seu próprio USB para telemetria.
