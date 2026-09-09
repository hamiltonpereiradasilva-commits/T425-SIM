# T425-SIM v2.8 — PIO physical bring-up

Esta versão fixa a topologia elétrica e prepara a camada PIO para o primeiro ensaio P00↔P01.

## Objetivo

Demonstrar primeiro um byte bidirecional entre dois RP2040, antes de habilitar framing/reliable-link completo.

## Sequência de bancada

```text
P00 GP0 TX ───────────────► P01 GP1 RX
P00 GP1 RX ◄─────────────── P01 GP0 TX
P00 GND ─────────────────── P01 GND
```

1. Gravar firmware de bring-up.
2. Conectar GND.
3. Conectar TX/RX cruzados.
4. Ligar P00 e P01.
5. Confirmar USB CDC de ambos.
6. Executar loopback local.
7. Executar P00↔P01.
8. Medir sinal com analisador lógico/osciloscópio.
9. Só depois ativar frame/CRC/ACK sobre o PIO.

## Critério de aceitação

- nenhum byte perdido em teste controlado;
- TX/RX coerentes;
- clock/baud medido dentro da tolerância definida;
- CRC passa em frames;
- timeout/retry funcionam após a integração.
