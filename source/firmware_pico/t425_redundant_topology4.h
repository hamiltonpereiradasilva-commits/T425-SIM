#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_route_candidates.h"

/*
 * Redundant physical topology for four nodes.
 *
 * Links:
 *   P00 <-> P01
 *   P00 <-> P02
 *   P01 <-> P03
 *   P02 <-> P03
 *
 * Two disjoint 2-hop paths exist between P00 and P03:
 *   A: P00-P01-P03
 *   B: P00-P02-P03
 *
 * This header defines candidates only; physical interface binding is a
 * separate concern and must not be inferred from the route table.
 */
static inline void t425_redundant_routes_init(uint8_t node_id,
                                               t425_route_set_t *r,
                                               uint8_t destination) {
    t425_route_set_init(r, destination);

    if (node_id == 0 && destination == 3) {
        t425_route_add(r, 1, 2); /* primary */
        t425_route_add(r, 2, 2); /* alternate */
    } else if (node_id == 3 && destination == 0) {
        t425_route_add(r, 1, 2); /* primary reverse */
        t425_route_add(r, 2, 2); /* alternate reverse */
    }
}
