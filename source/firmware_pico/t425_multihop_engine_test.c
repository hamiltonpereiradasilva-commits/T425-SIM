#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_forward_engine.h"

typedef struct {
    t425_router_t *routers[3];
    unsigned sends;
} test_ctx_t;

static bool test_send(uint8_t next_hop, const uint8_t *payload,
                      uint8_t payload_len, void *ctx) {
    test_ctx_t *t = (test_ctx_t *)ctx;
    (void)payload;
    (void)payload_len;
    t->sends++;
    return next_hop < 3u;
}

int main(void) {
    stdio_init_all();
    sleep_ms(1000);

    t425_router_t p0, p1, p2;
    t425_router_init(&p0, 0);
    t425_router_init(&p1, 1);
    t425_router_init(&p2, 2);

    t425_router_set_route(&p0, 2, 1);
    t425_router_set_route(&p1, 2, 2);

    test_ctx_t ctx = {{&p0, &p1, &p2}, 0};
    t425_forward_engine_t e0, e1, e2;
    t425_forward_engine_init(&e0, &p0, test_send, &ctx);
    t425_forward_engine_init(&e1, &p1, test_send, &ctx);
    t425_forward_engine_init(&e2, &p2, test_send, &ctx);

    uint8_t payload[8] = { 'T','4','2','5','-','O','K','!' };
    t425_forward_hdr_t h;
    t425_forward_build(&h, 0, 1, 2, 4, sizeof(payload));

    printf("T425_MULTIHOP V2.45\r\n");

    t425_forward_result_t a0 = t425_forward_process(&e0, &h, payload);
    printf("P0 RESULT=%u DST=%u HOPS=%u TTL=%u\r\n",
           a0, h.dst, h.hop_count, h.ttl);

    t425_forward_result_t a1 = t425_forward_process(&e1, &h, payload);
    printf("P1 RESULT=%u DST=%u HOPS=%u TTL=%u\r\n",
           a1, h.dst, h.hop_count, h.ttl);

    t425_forward_result_t a2 = t425_forward_process(&e2, &h, payload);
    printf("P2 RESULT=%u DST=%u HOPS=%u TTL=%u\r\n",
           a2, h.dst, h.hop_count, h.ttl);

    printf("SENDS=%u P0_FWD=%lu P1_FWD=%lu P2_DELIVER=%lu\r\n",
           ctx.sends,
           (unsigned long)e0.forwarded,
           (unsigned long)e1.forwarded,
           (unsigned long)e2.delivered);

    while (1) sleep_ms(1000);
}
