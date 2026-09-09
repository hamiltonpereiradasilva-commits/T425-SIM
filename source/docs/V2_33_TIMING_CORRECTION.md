# T425-SIM v2.33 — correção da temporização PIO

Antes de avançar, foi encontrada uma inconsistência na v2.32: o divisor de clock havia sido calculado como se o enquadramento ocupasse 10 clocks PIO por símbolo, mas o programa PIO executava uma sequência com número diferente de instruções/ciclos.

Isso foi corrigido explicitamente.

## Novo contrato

```text
serial bit rate = baud
PIO clocks/bit  = 8
PIO frequency   = baud × 8
clkdiv          = sys_hz / (baud × 8)
```

A configuração agora declara:

```text
DATA       = 8 bits
PARITY     = 0
STOP       = 1
PIO/bit    = 8 clocks
```

O firmware USB também imprime `PIO_CYCLES` para que a configuração possa ser conferida na bancada.

## Importante

Esta é uma correção de modelo temporal de software. Ainda não constitui validação elétrica. O sinal deve ser medido no analisador lógico antes de declarar o PHY aprovado.
