#include "t425_pio_phy.h"
#include "t425_pio_tx.pio.h"
#include "t425_pio_rx.pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include <string.h>

static float div_for(uint32_t sys_hz, uint32_t baud) {
    return (float)sys_hz / ((float)baud * 8.0f);
}

bool t425_pio_phy_init(t425_pio_phy_t *p, PIO pio, uint sm_tx, uint sm_rx,
                       uint tx_pin, uint rx_pin, uint32_t bit_rate) {
    if (!p || !pio || sm_tx >= 4 || sm_rx >= 4 || sm_tx == sm_rx || !bit_rate)
        return false;
    memset(p, 0, sizeof(*p));
    p->pio = pio; p->sm_tx = sm_tx; p->sm_rx = sm_rx;
    p->tx_pin = tx_pin; p->rx_pin = rx_pin; p->bit_rate = bit_rate;
    p->sys_hz = clock_get_hz(clk_sys);

    int ot = pio_add_program(pio, &t425_pio_tx_program);
    if (ot < 0) return false;
    int or = pio_add_program(pio, &t425_pio_rx_program);
    if (or < 0) return false;

    pio_sm_config ct = t425_pio_tx_program_get_default_config((uint)ot);
    sm_config_set_out_pins(&ct, tx_pin, 1);
    sm_config_set_sideset_pins(&ct, tx_pin);
    sm_config_set_out_shift(&ct, true, false, 32);
    sm_config_set_fifo_join(&ct, PIO_FIFO_JOIN_TX);
    sm_config_set_clkdiv(&ct, div_for(p->sys_hz, bit_rate));
    pio_sm_set_pins_with_mask(pio, sm_tx, 1u << tx_pin, 1u << tx_pin);
    pio_sm_set_pindirs_with_mask(pio, sm_tx, 1u << tx_pin, 1u << tx_pin);
    pio_gpio_init(pio, tx_pin);
    pio_sm_init(pio, sm_tx, (uint)ot, &ct);

    pio_sm_config cr = t425_pio_rx_program_get_default_config((uint)or);
    sm_config_set_in_pins(&cr, rx_pin);
    sm_config_set_jmp_pin(&cr, rx_pin);
    sm_config_set_in_shift(&cr, true, true, 8);
    sm_config_set_fifo_join(&cr, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv(&cr, div_for(p->sys_hz, bit_rate));
    pio_sm_set_consecutive_pindirs(pio, sm_rx, rx_pin, 1, false);
    pio_gpio_init(pio, rx_pin);
    gpio_pull_up(rx_pin);
    pio_sm_init(pio, sm_rx, (uint)or, &cr);

    pio_sm_set_enabled(pio, sm_rx, true);
    pio_sm_set_enabled(pio, sm_tx, true);
    p->initialized = true;
    return true;
}

void t425_pio_phy_reset_fifos(t425_pio_phy_t *p) {
    if (!p || !p->initialized) return;
    pio_sm_clear_fifos(p->pio, p->sm_tx);
    pio_sm_clear_fifos(p->pio, p->sm_rx);
}

bool t425_pio_phy_write(t425_pio_phy_t *p, const uint8_t *d, uint16_t n) {
    if (!p || !p->initialized || !d || !n) return false;
    for (uint16_t i = 0; i < n; ++i)
        pio_sm_put_blocking(p->pio, p->sm_tx, d[i]);
    return true;
}

uint16_t t425_pio_phy_read(t425_pio_phy_t *p, uint8_t *d, uint16_t max) {
    if (!p || !p->initialized || !d || !max) return 0;
    uint16_t n = 0;
    while (n < max && !pio_sm_is_rx_fifo_empty(p->pio, p->sm_rx)) {
        uint32_t w = pio_sm_get(p->pio, p->sm_rx);
        d[n++] = (uint8_t)(w >> 24); /* right-shift RX: 8-bit sample is upper byte */
    }
    return n;
}
