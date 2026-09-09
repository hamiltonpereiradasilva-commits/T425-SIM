#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    T425_LINK_UP = 0,
    T425_LINK_DEGRADED = 1,
    T425_LINK_DOWN = 2
} t425_link_state_t;

typedef struct {
    uint8_t peer;
    uint8_t retry_limit;
    uint8_t consecutive_timeouts;
    uint32_t tx_ok;
    uint32_t tx_retry;
    uint32_t timeout_count;
    t425_link_state_t state;
} t425_link_health_t;

void t425_link_health_init(t425_link_health_t *h, uint8_t peer, uint8_t retry_limit);
void t425_link_health_on_ack(t425_link_health_t *h);
void t425_link_health_on_timeout(t425_link_health_t *h);
bool t425_link_health_is_up(const t425_link_health_t *h);
