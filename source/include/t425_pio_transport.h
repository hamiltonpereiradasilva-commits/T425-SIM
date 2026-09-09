#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_PIO_LINKS 2
#define T425_PIO_BAUD 2000000u
typedef struct { uint8_t tx_pin,rx_pin,pio_index,tx_sm,rx_sm; bool initialized; } t425_pio_link_cfg_t;
bool t425_pio_transport_init(uint8_t link,const t425_pio_link_cfg_t*cfg);
bool t425_pio_transport_write(uint8_t link,const uint8_t*data,uint16_t len);
uint16_t t425_pio_transport_read(uint8_t link,uint8_t*data,uint16_t max_len);
