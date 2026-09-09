#pragma once
#include <stdint.h>
#include <stdbool.h>
void t425_pio_links_init(void);
bool t425_pio_link_send(uint8_t link,const uint8_t*data,uint16_t len);
bool t425_pio_link_receive(uint8_t link,uint8_t*data,uint16_t*len);
