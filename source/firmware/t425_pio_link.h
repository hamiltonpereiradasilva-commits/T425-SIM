#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

typedef struct {
    PIO pio;
    uint sm_tx, sm_rx;
    uint pin_tx, pin_rx;
    uint baud;
    int tx_program_offset;
    int rx_program_offset;
    bool initialized;
} t425_pio_link_t;

bool t425_pio_link_init(t425_pio_link_t *l, PIO pio, uint sm_tx, uint sm_rx,
                        uint pin_tx, uint pin_rx, uint baud);
bool t425_pio_link_put(t425_pio_link_t *l, uint8_t byte);
bool t425_pio_link_get(t425_pio_link_t *l, uint8_t *byte);
