#include "t425_router.h"
#include <string.h>

void t425_router_init(t425_router_t *r, uint8_t node_id) {
    memset(r, 0, sizeof(*r));
    r->node_id = node_id;
    r->table[node_id].node_id = node_id;
    r->table[node_id].next_hop = node_id;
    r->table[node_id].valid = true;
}

bool t425_router_set_route(t425_router_t *r, uint8_t destination, uint8_t next_hop) {
    if (!r || destination >= T425_ROUTER_MAX_NODES ||
        next_hop >= T425_ROUTER_MAX_NODES) return false;
    r->table[destination].node_id = destination;
    r->table[destination].next_hop = next_hop;
    r->table[destination].valid = true;
    return true;
}

t425_route_action_t t425_router_lookup(const t425_router_t *r,
                                       uint8_t destination, uint8_t *next_hop) {
    if (!r) return T425_ROUTE_DROP;

    if (destination == r->node_id || destination == T425_ROUTE_LOCAL) {
        return T425_ROUTE_LOCAL_DELIVER;
    }

    if (destination >= T425_ROUTER_MAX_NODES ||
        !r->table[destination].valid) {
        return T425_ROUTE_DROP;
    }

    if (next_hop) *next_hop = r->table[destination].next_hop;
    return T425_ROUTE_FORWARD;
}
