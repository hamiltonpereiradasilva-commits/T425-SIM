#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_ROUTER_MAX_NODES 16u
#define T425_ROUTE_LOCAL      0x00u
#define T425_ROUTE_BROADCAST  0xFFu

typedef enum {
    T425_ROUTE_DROP = 0,
    T425_ROUTE_LOCAL_DELIVER = 1,
    T425_ROUTE_FORWARD = 2
} t425_route_action_t;

typedef struct {
    uint8_t node_id;
    uint8_t next_hop;
    bool valid;
} t425_route_entry_t;

typedef struct {
    uint8_t node_id;
    t425_route_entry_t table[T425_ROUTER_MAX_NODES];
    uint32_t local_deliveries;
    uint32_t forwarded;
    uint32_t dropped;
    uint32_t no_route;
} t425_router_t;

void t425_router_init(t425_router_t *r, uint8_t node_id);
bool t425_router_set_route(t425_router_t *r, uint8_t destination, uint8_t next_hop);
t425_route_action_t t425_router_lookup(const t425_router_t *r,
                                       uint8_t destination, uint8_t *next_hop);
