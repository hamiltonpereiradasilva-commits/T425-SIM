# T425-SIM v2.7 — Link Stack

A v2.7 integra as camadas anteriores numa pilha lógica única:

```text
Application / OCCAM
        │
Reliable Link
        │
Frame Builder
        │
CRC16
        │
TX buffer
        │
PIO
        │
================ physical link ================
        │
PIO
        │
RX buffer
        │
Frame Parser
        │
CRC16
        │
Reliable Link
        │
Application
```

A API de integração é:

- `t425_link_stack_prepare()`
- `t425_link_stack_receive()`

A entrega atual é um modelo de host determinístico. O transporte físico PIO será conectado a estas funções após a validação do enlace P00↔P01.
