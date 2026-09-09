#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

typedef struct {
    PIO pio;
    uint sm_tx;
    uint sm_rx;
    uint pin_tx;
    uint pin_rx;
    uint baud;
    bool initialized;
} t425_link_t;

bool t425_link_init(t425_link_t *l, PIO pio, uint sm_tx, uint sm_rx,
                    uint pin_tx, uint pin_rx, uint baud);
bool t425_link_write_byte(t425_link_t *l, uint8_t b);
bool t425_link_read_byte(t425_link_t *l, uint8_t *b);
