# T425-SIM v2.37 — PHY PIO canonicalizada

A v2.37 abandona a lógica experimental das versões anteriores e passa a usar como referência direta os exemplos oficiais `pio/uart_tx` e `pio/uart_rx` do `raspberrypi/pico-examples`.

## TX

- 8N1
- LSB first
- 8 PIO cycles/bit
- `side-set` no mesmo GPIO do TX
- `PIO_FIFO_JOIN_TX`
- `sm_config_set_out_shift(..., true, false, 32)`

## RX

- espera o start bit
- espera o centro do primeiro bit com `[10]`
- 8 amostras, cada iteração totalizando 8 ciclos
- verifica stop bit
- marca framing error com IRQ 4
- `autopush` a cada 8 bits
- `PIO_FIFO_JOIN_RX`

## Correção crítica do software

Com `shift_right=true` e autopush de 8 bits, o byte recebido fica justificado no byte superior do FIFO. Portanto `t425_pio_phy_read()` agora extrai:

```c
(uint8_t)(pio_sm_get(...) >> 24)
```

Isso evita o erro das versões anteriores, que convertiam diretamente o word de 32 bits para `uint8_t`.

## Referência externa

A estrutura temporal segue os exemplos oficiais da Raspberry Pi para UART em PIO.
