#pragma once
#include <stdint.h>
#include "t425_router.h"

/* Single firmware, four-node linear topology:
   P00 <-> P01 <-> P02 <-> P03 */
static inline void t425_topology4_install(t425_router_t *r) {
    if (!r) return;
    switch (r->node_id) {
        case 0:
            t425_router_set_route(r, 1, 1);
            t425_router_set_route(r, 2, 1);
            t425_router_set_route(r, 3, 1);
            break;
        case 1:
            t425_router_set_route(r, 0, 0);
            t425_router_set_route(r, 2, 2);
            t425_router_set_route(r, 3, 2);
            break;
        case 2:
            t425_router_set_route(r, 0, 1);
            t425_router_set_route(r, 1, 1);
            t425_router_set_route(r, 3, 3);
            break;
        case 3:
            t425_router_set_route(r, 0, 2);
            t425_router_set_route(r, 1, 2);
            t425_router_set_route(r, 2, 2);
            break;
        default:
            break;
    }
}
