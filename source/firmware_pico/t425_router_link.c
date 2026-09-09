#include "t425_router_link.h"

void t425_router_link_init(t425_router_link_t *x, uint8_t node_id,
                           uint8_t destination) {
    x->node_id = node_id;
    t425_route_set_init(&x->route, destination);
    t425_interfaces_init(&x->interfaces);
}

bool t425_router_link_add_route(t425_router_link_t *x, uint8_t next_hop,
                                uint8_t metric) {
    return t425_route_add(&x->route, next_hop, metric);
}

bool t425_router_link_resolve(const t425_router_link_t *x,
                              const bool *peer_up, uint8_t peer_count,
                              uint8_t *next_hop, int *if_id) {
    if (!x || !next_hop || !if_id) return false;
    if (!t425_route_select(&x->route, peer_up, peer_count, next_hop))
        return false;
    *if_id = t425_interface_for_peer(&x->interfaces, *next_hop);
    if (*if_id < 0) return false;
    return true;
}
