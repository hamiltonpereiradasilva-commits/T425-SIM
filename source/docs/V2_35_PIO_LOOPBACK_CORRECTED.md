# T425-SIM v2.35 — PIO loopback corrigido

Foi feita uma revisão do TX/RX antes de qualquer teste físico.

## Correções

1. TX usa explicitamente o `side-set` como linha serial.
2. `sm_config_set_sideset_pins()` agora é chamado.
3. TX usa `out pins, 1` para os dados e side-set para start/stop.
4. RX usa `sm_config_set_in_shift(..., false, true, 8)`.
5. O RX aguarda uma janela de 8 clocks antes da primeira amostra, aproximando a amostragem do centro do primeiro bit de dados para o enlace de 8 clocks/bit.
6. Foi criado um firmware de loopback independente de Router/OCCAM.

## Loopback

```text
Pico único

GP2 TX ───────── jumper ───────── GP3 RX
GND  ───────────────────────────── GND
```

O firmware transmite:

```text
55 AA 00 FF 12 34 C3 3C
```

e compara os bytes recebidos.

## Critério

`ERR=0` em teste prolongado.

Importante: isso é um teste físico real somente quando executado no Pico. A análise do código não substitui a medição elétrica.
