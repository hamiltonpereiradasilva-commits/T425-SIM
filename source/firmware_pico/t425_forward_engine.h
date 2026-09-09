#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_forward.h"
#include "t425_router.h"

typedef enum {
    T425_FWD_DROP = 0,
    T425_FWD_DELIVER = 1,
    T425_FWD_SEND_NEXT = 2
} t425_forward_result_t;

typedef bool (*t425_forward_send_fn)(uint8_t next_hop,
                                     const uint8_t *payload,
                                     uint8_t payload_len,
                                     void *ctx);

typedef struct {
    t425_router_t *router;
    t425_forward_send_fn send_next;
    void *send_ctx;
    uint32_t delivered;
    uint32_t forwarded;
    uint32_t ttl_drops;
    uint32_t route_drops;
} t425_forward_engine_t;

void t425_forward_engine_init(t425_forward_engine_t *e,
                              t425_router_t *router,
                              t425_forward_send_fn send_next,
                              void *ctx);

t425_forward_result_t t425_forward_process(t425_forward_engine_t *e,
                                            t425_forward_hdr_t *h,
                                            const uint8_t *payload);
