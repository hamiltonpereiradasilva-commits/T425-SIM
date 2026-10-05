# F_PHY-SYS-01 — Infraestrutura de sinais físicos

Esta etapa transporta níveis digitais entre GPIOs RP2040 e a fronteira
portátil t425_core_phy_t. Ela não implementa semântica, FSM, rendezvous,
scheduler, ACK físico ou comportamento de memória no CORE.

## Mapa GPIO canônico

| Sinal | Direção | GPIO | Estado |
|---|---:|---:|---|
| Reset | entrada | GP4 | reservado |
| Analyse | entrada | GP5 | reservado |
| ErrorIn | entrada | GP6 | reservado |
| BootFromROM | entrada | GP7 | reservado |
| EventReq | entrada | GP8 | reservado |
| DisableIntRAM | entrada | GP9 | reservado |
| ProcSpeedSelect0 | entrada | GP10 | reservado |
| ProcSpeedSelect1 | entrada | GP11 | reservado |
| ProcSpeedSelect2 | entrada | GP12 | reservado |
| LinkSpecial | entrada | GP20 | existente, preservado |
| Link0Special | entrada | GP21 | existente, preservado |
| Link123Special | entrada | GP22 | existente, preservado |
| Error | saída | GP13 | reservado |
| EventAck | saída | GP14 | reservado |
| EventWaiting | saída | GP15 | reservado |

Os links continuam em GP0/1, GP2/3, GP17/16 e GP19/18. Não há colisão.
Nenhuma PIO state machine nova foi criada.

## Implementação

O contrato portátil acrescenta estruturas lógicas de entradas/saídas e
callbacks opcionais read_system_inputs e write_system_outputs. O IPC
mantém um snapshot de entradas e saídas; o serviço atualiza o snapshot a
partir do adapter e aplica as saídas ao adapter. O adapter configura e lê
GPIOs somente como níveis digitais.

Os callbacks são opcionais para preservar testes e consumidores existentes.
O CORE não interpreta esses valores nesta etapa.

## Critérios

- GPIO → adapter → IPC → contrato portátil: implementado.
- contrato portátil → IPC → adapter → GPIO: implementado.
- Link0..3, ACK físico e seleção de velocidade existentes: preservados.
- Sem debounce, protocolo, scheduler, FSM ou consequências arquiteturais.

The numeric RX/TX order of Links 2 and 3 is intentionally mirrored physically relative to Links 0 and 1. This does not invert TX/RX semantics. GP23..GP25 remain reserved and unused by the T425 signal interface.
