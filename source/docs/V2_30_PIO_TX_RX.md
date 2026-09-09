# T425-SIM v2.30 — PIO TX/RX

A camada física agora possui programas PIO separados para TX e RX.

## Enquadramento

```text
IDLE  START  D0 D1 D2 D3 D4 D5 D6 D7  STOP
 1      0    <---- LSB first ---->     1
```

O TX e o RX usam uma máquina de estados dedicada. O baud configurado é interpretado como taxa de bits, com 10 clocks PIO por byte (start + 8 data + stop).

## Limitação importante

Os arquivos `.pio.h` incluídos no pacote são placeholders de integração para manter a estrutura do projeto. Em uma build real do Pico SDK, devem ser gerados pelo `pioasm` a partir dos `.pio`.

Além disso, a taxa de clock do RP2040 foi fixada em 125 MHz nesta primeira integração; isso deve ser derivado do clock real do sistema na próxima revisão.

## Estado da validação

Esta versão é **integração de fonte**, não validação elétrica. O RX/TX precisa ser testado em dois Pico reais e, idealmente, com analisador lógico.
