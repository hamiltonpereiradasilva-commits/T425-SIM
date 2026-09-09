# T425-SIM v2.4 — Driver PIO

A v2.4 começa a integração do transporte com as máquinas de estado PIO do RP2040.

## Separação

```text
C application
     │
     ▼
T425 link driver
     │
     ▼
PIO TX/RX state machines
     │
     ▼
GPIO
     │
     ▼
P00 <──────> P01
```

O driver mantém a API mínima:
- `t425_link_init()`
- `t425_link_write_byte()`
- `t425_link_read_byte()`

## Limitação deliberada

O programa PIO e a API estão preparados para bring-up, mas a temporização, polaridade e sincronismo ainda precisam ser medidos no hardware. Não considerar a v2.4 como enlace de produção.
