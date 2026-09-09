#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_route_candidates.h"
#include "t425_interfaces.h"

typedef struct {
    uint8_t node_id;
    t425_route_set_t route;
    t425_interface_table_t interfaces;
} t425_router_link_t;

void t425_router_link_init(t425_router_link_t *x, uint8_t node_id,
                           uint8_t destination);
bool t425_router_link_add_route(t425_router_link_t *x, uint8_t next_hop,
                                uint8_t metric);
bool t425_router_link_resolve(const t425_router_link_t *x,
                              const bool *peer_up, uint8_t peer_count,
                              uint8_t *next_hop, int *if_id);
