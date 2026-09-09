#include "t425_forward_engine.h"

void t425_forward_engine_init(t425_forward_engine_t *e,
                              t425_router_t *router,
                              t425_forward_send_fn send_next,
                              void *ctx) {
    e->router = router;
    e->send_next = send_next;
    e->send_ctx = ctx;
    e->delivered = 0;
    e->forwarded = 0;
    e->ttl_drops = 0;
    e->route_drops = 0;
}

t425_forward_result_t t425_forward_process(t425_forward_engine_t *e,
                                           t425_forward_hdr_t *h,
                                           const uint8_t *payload) {
    if (!e || !h || !e->router) return T425_FWD_DROP;
    if (!t425_forward_forwardable(h)) {
        e->ttl_drops++;
        return T425_FWD_DROP;
    }

    uint8_t hop = 0xFFu;
    t425_route_action_t action =
        t425_router_lookup(e->router, h->final_dst, &hop);

    if (action == T425_ROUTE_LOCAL_DELIVER) {
        e->delivered++;
        return T425_FWD_DELIVER;
    }

    if (action != T425_ROUTE_FORWARD || !e->send_next) {
        e->route_drops++;
        return T425_FWD_DROP;
    }

    if (!t425_forward_forwardable(h)) {
        e->ttl_drops++;
        return T425_FWD_DROP;
    }

    t425_forward_advance(h, hop);

    if (!e->send_next(hop, payload, h->payload_len, e->send_ctx)) {
        e->route_drops++;
        return T425_FWD_DROP;
    }

    e->forwarded++;
    return T425_FWD_SEND_NEXT;
}
