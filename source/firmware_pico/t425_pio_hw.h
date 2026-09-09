#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_HW_LINKS 2
typedef struct {
    uint8_t tx_pin, rx_pin, pio_index, tx_sm, rx_sm;
    uint32_t baud;
} t425_pio_hw_cfg_t;
bool t425_pio_hw_init(const t425_pio_hw_cfg_t cfg[T425_HW_LINKS]);
bool t425_pio_hw_write(uint8_t link, const uint8_t *data, uint16_t len);
uint16_t t425_pio_hw_read(uint8_t link, uint8_t *data, uint16_t max_len);
bool t425_pio_hw_ready(uint8_t link);
