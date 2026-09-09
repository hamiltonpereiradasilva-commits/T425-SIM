#pragma once
#include <stdint.h>
#define T425_NET_NODES 4
typedef struct { uint8_t node; uint8_t link_up_mask; uint32_t tx; uint32_t rx; } t425_net_status_t;
uint8_t t425_net_next(uint8_t node,uint8_t link);
void t425_net_ring_defaults(uint8_t node,uint8_t*left,uint8_t*right);
