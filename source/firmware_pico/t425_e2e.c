#include "t425_e2e.h"

void t425_e2e_init(t425_e2e_t *e, uint8_t max_attempts) {
    e->tx_id = 0;
    e->src = 0;
    e->final_dst = 0;
    e->attempts = 0;
    e->max_attempts = max_attempts;
    e->state = T425_E2E_IDLE;
    e->delivery_ack_sent = false;
    e->duplicate_delivery = false;
}

bool t425_e2e_start(t425_e2e_t *e, uint8_t tx_id,
                   uint8_t src, uint8_t final_dst) {
    if (!e || e->state == T425_E2E_PENDING) return false;
    e->tx_id = tx_id;
    e->src = src;
    e->final_dst = final_dst;
    e->attempts = 1;
    e->state = T425_E2E_PENDING;
    e->delivery_ack_sent = false;
    e->duplicate_delivery = false;
    return true;
}

void t425_e2e_on_delivery(t425_e2e_t *e, uint8_t tx_id) {
    if (!e || e->state != T425_E2E_PENDING) return;
    if (e->tx_id == tx_id) {
        e->state = T425_E2E_DELIVERED;
        e->delivery_ack_sent = true;
    }
}

bool t425_e2e_retry(t425_e2e_t *e) {
    if (!e || e->state != T425_E2E_PENDING) return false;
    if (e->attempts >= e->max_attempts) {
        e->state = T425_E2E_FAILED;
        return false;
    }
    e->attempts++;
    return true;
}

bool t425_e2e_destination_accept(uint8_t tx_id, uint8_t *last_tx_id,
                                 bool *valid_last) {
    if (!last_tx_id || !valid_last) return false;
    if (*valid_last && *last_tx_id == tx_id) return false;
    *last_tx_id = tx_id;
    *valid_last = true;
    return true;
}
