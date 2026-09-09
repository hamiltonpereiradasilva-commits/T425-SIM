#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    T425_E2E_IDLE = 0,
    T425_E2E_PENDING = 1,
    T425_E2E_DELIVERED = 2,
    T425_E2E_FAILED = 3
} t425_e2e_state_t;

typedef struct {
    uint8_t tx_id;
    uint8_t src;
    uint8_t final_dst;
    uint8_t attempts;
    uint8_t max_attempts;
    t425_e2e_state_t state;
    bool delivery_ack_sent;
    bool duplicate_delivery;
} t425_e2e_t;

void t425_e2e_init(t425_e2e_t *e, uint8_t max_attempts);
bool t425_e2e_start(t425_e2e_t *e, uint8_t tx_id,
                    uint8_t src, uint8_t final_dst);
void t425_e2e_on_delivery(t425_e2e_t *e, uint8_t tx_id);
bool t425_e2e_retry(t425_e2e_t *e);

/* Destination-side duplicate suppression. */
bool t425_e2e_destination_accept(uint8_t tx_id, uint8_t *last_tx_id,
                                  bool *valid_last);
