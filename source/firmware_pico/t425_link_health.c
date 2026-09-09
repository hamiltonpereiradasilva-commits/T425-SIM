#include "t425_link_health.h"

void t425_link_health_init(t425_link_health_t *h, uint8_t peer, uint8_t retry_limit) {
    h->peer = peer;
    h->retry_limit = retry_limit;
    h->consecutive_timeouts = 0;
    h->tx_ok = 0;
    h->tx_retry = 0;
    h->timeout_count = 0;
    h->state = T425_LINK_UP;
}

void t425_link_health_on_ack(t425_link_health_t *h) {
    h->tx_ok++;
    h->consecutive_timeouts = 0;
    h->state = T425_LINK_UP;
}

void t425_link_health_on_timeout(t425_link_health_t *h) {
    h->timeout_count++;
    h->consecutive_timeouts++;
    if (h->consecutive_timeouts >= h->retry_limit)
        h->state = T425_LINK_DOWN;
    else
        h->state = T425_LINK_DEGRADED;
}

bool t425_link_health_is_up(const t425_link_health_t *h) {
    return h && h->state != T425_LINK_DOWN;
}
