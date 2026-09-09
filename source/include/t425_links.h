#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_LINKS 4
typedef struct {uint32_t tx_packets,rx_packets,tx_errors,rx_errors;} t425_link_stats_t;
void t425_links_init(void);
bool t425_link_send(uint8_t link,const uint8_t *data,uint16_t len);
bool t425_link_receive(uint8_t link,uint8_t *data,uint16_t *len);
void t425_link_get_stats(uint8_t link,t425_link_stats_t *s);
