# T425-SIM v2.2 — Pinout de referência RP2040

## Topologia de bancada

Quatro Raspberry Pi Pico:
- P00 = nó 0
- P01 = nó 1
- P02 = nó 2
- P03 = nó 3

Cada nó utiliza dois links seriais PIO, permitindo uma topologia linear:

```text
P00 <----> P01 <----> P02 <----> P03
```

## Sinais por link

Para cada enlace:
- TX
- RX
- GND

A alimentação dos Pico deve ser tratada separadamente; o GND precisa ser comum quando os níveis lógicos forem compartilhados.

## Pinagem proposta

| Nó | Link esquerdo | Link direito |
|---|---|---|
| P00 | — | GP0 TX / GP1 RX |
| P01 | GP2 TX / GP3 RX | GP0 TX / GP1 RX |
| P02 | GP2 TX / GP3 RX | GP0 TX / GP1 RX |
| P03 | GP2 TX / GP3 RX | — |

A pinagem é uma proposta de bancada, não uma validação elétrica da placa específica. Antes da energização deve-se conferir o esquema/datasheet do Pico utilizado.

## Regra elétrica

Não conectar GPIO de fontes com níveis incompatíveis. Para Pico/RP2040, manter o enlace em lógica de 3,3 V e usar GND comum.
