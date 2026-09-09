#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_frame.h"

typedef enum {
    T425_TX_IDLE=0,
    T425_TX_WAIT_ACK,
    T425_TX_RETRY,
    T425_TX_FAILED
} t425_tx_state_t;

typedef struct {
    t425_tx_state_t state;
    uint8_t seq;
    uint8_t retries;
    uint8_t max_retries;
    uint32_t timeout_ticks;
    uint32_t deadline;
    uint32_t acked;
    uint32_t failed;
    uint32_t timeouts;
    uint32_t retries_sent;
} t425_ack_ctrl_t;

void t425_ack_init(t425_ack_ctrl_t *a, uint8_t max_retries, uint32_t timeout_ticks);
bool t425_ack_start(t425_ack_ctrl_t *a, uint8_t seq, uint32_t now);
bool t425_ack_receive(t425_ack_ctrl_t *a, uint8_t seq);
bool t425_ack_poll(t425_ack_ctrl_t *a, uint32_t now);
bool t425_ack_can_retry(const t425_ack_ctrl_t *a);
