# T425-SIM v1.1 — Bring-up físico

## Fase 1 — um link
P00 ↔ P01.

1. Alimentar os dois Pico separadamente ou por fonte comum adequada.
2. Conectar GND.
3. Conectar TX0→RX0 e RX0→TX0.
4. Carregar firmware de loopback/echo.
5. Validar bytes com analisador lógico.

## Fase 2 — pacote
Enviar `PING` P00→P01 e esperar `PONG`.

## Fase 3 — dados
Enviar `DATA` com `SRC`, `DST`, `SEQ`, `LEN`, payload e CRC.

## Fase 4 — anel
P00→P01→P02→P03→P00.

## Fase 5 — OCCAM
Mapear:

`CHAN c0` → link lógico → packet DATA → link físico.

## Critério de aceite

- zero erros em 10.000 pacotes de teste;
- detecção de CRC inválido;
- recuperação após byte descartado;
- timeout de link;
- contador TX/RX por nó.
