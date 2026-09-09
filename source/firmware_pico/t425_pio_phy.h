#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

typedef struct {
    PIO pio;
    uint sm_tx, sm_rx;
    uint tx_pin, rx_pin;
    uint32_t sys_hz, bit_rate;
    bool initialized;
} t425_pio_phy_t;

bool t425_pio_phy_init(t425_pio_phy_t*, PIO, uint sm_tx, uint sm_rx,
                       uint tx_pin, uint rx_pin, uint32_t bit_rate);
void t425_pio_phy_reset_fifos(t425_pio_phy_t*);
bool t425_pio_phy_write(t425_pio_phy_t*, const uint8_t*, uint16_t);
uint16_t t425_pio_phy_read(t425_pio_phy_t*, uint8_t*, uint16_t);
