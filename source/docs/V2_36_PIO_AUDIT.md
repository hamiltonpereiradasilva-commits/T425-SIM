# T425-SIM v2.36 — auditoria final do loopback PIO

A v2.36 simplifica a PHY para evitar ambiguidades introduzidas pelo side-set no TX.

## TX

O TX usa `SET` para start/stop e `OUT` para os bits de dados.

```text
START = 0
D0..D7 = LSB first
STOP = 1
IDLE = 1
```

O OSR é deslocado para a direita, portanto o bit 0 é o primeiro bit emitido.

## RX

O RX:

1. espera o start `0`;
2. executa atraso de meio bit;
3. amostra 8 bits;
4. autopush em 8 bits;
5. espera o retorno ao nível idle.

## Configuração explícita

O código configura explicitamente:

- `out_pins`;
- `set_pins`;
- `in_pins`;
- `out_shift`;
- `in_shift`;
- `clkdiv`;
- direção dos GPIOs;
- estado inicial do TX.

Isso é necessário porque o SDK define defaults próprios para `pio_sm_config`, inclusive shift para a direita, autopush/autopull desabilitados e threshold 32. Portanto não dependemos dos defaults. A documentação oficial confirma esses defaults e as APIs de configuração. citeturn0search0turn0search1

## Estado

Esta versão está pronta para **compilação e teste de bancada**, mas continua sem validação elétrica nesta sessão.
