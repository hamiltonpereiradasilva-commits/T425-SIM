#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_router.h"
#include "t425_reliable.h"

typedef bool (*t425_emit_fn)(uint8_t link, const uint8_t *data, uint16_t len, void *ctx);

typedef struct {
    t425_router_t *router;
    t425_reliable_t reliable[8];
    t425_emit_fn emit;
    void *emit_ctx;
    uint8_t default_ttl;
    uint8_t next_sequence;
    uint32_t sent;
    uint32_t delivered;
    uint32_t failed;
} t425_application_t;

void t425_app_init(t425_application_t *a, t425_router_t *r, t425_emit_fn emit, void *ctx);
bool t425_app_send(t425_application_t *a, uint8_t destination,
                   const uint8_t *payload, uint8_t length, uint32_t now);
void t425_app_poll(t425_application_t *a, uint32_t now);
