#include "t425_route_candidates.h"

void t425_route_set_init(t425_route_set_t *r, uint8_t destination) {
    r->destination = destination;
    r->count = 0;
    for (unsigned i=0; i<T425_MAX_ROUTE_CANDIDATES; ++i) {
        r->candidates[i].next_hop = 0xFF;
        r->candidates[i].metric = 0xFF;
        r->candidates[i].valid = false;
    }
}

bool t425_route_add(t425_route_set_t *r, uint8_t next_hop, uint8_t metric) {
    if (!r || r->count >= T425_MAX_ROUTE_CANDIDATES) return false;
    r->candidates[r->count++] = (t425_route_candidate_t){
        .next_hop=next_hop, .metric=metric, .valid=true
    };
    return true;
}

bool t425_route_select(const t425_route_set_t *r,
                       const bool *peer_up, uint8_t peer_count,
                       uint8_t *next_hop) {
    if (!r || !peer_up || !next_hop) return false;

    bool found=false;
    uint8_t best=0xFF, best_metric=0xFF;
    for (uint8_t i=0; i<r->count; ++i) {
        const t425_route_candidate_t *c=&r->candidates[i];
        if (!c->valid || c->next_hop >= peer_count || !peer_up[c->next_hop])
            continue;
        if (!found || c->metric < best_metric) {
            found=true; best=c->next_hop; best_metric=c->metric;
        }
    }
    if (!found) return false;
    *next_hop=best;
    return true;
}
