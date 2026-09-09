#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_packet.h"

#define T425_ROUTER_MAX_ROUTES 16
#define T425_ROUTER_MAX_HOPS 8

typedef struct {
    uint8_t dst;
    uint8_t next_hop;
    uint8_t metric;
    bool valid;
} t425_route_t;

typedef struct {
    uint8_t node_id;
    t425_route_t routes[T425_ROUTER_MAX_ROUTES];
    uint8_t route_count;
} t425_router_t;

void t425_router_init(t425_router_t *r,uint8_t node_id);
bool t425_router_add_route(t425_router_t *r,uint8_t dst,uint8_t next_hop,uint8_t metric);
bool t425_router_lookup(const t425_router_t *r,uint8_t dst,uint8_t *next_hop);
bool t425_router_forward(const t425_router_t *r,const t425_link_packet_t *in,
                         t425_link_packet_t *out);
