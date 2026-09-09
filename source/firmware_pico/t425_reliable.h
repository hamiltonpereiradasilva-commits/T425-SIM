#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_RL_MAX_PAYLOAD 48u
#define T425_RL_MAX_RETRIES 5u

typedef enum {
    T425_RL_IDLE=0,
    T425_RL_WAIT_ACK,
    T425_RL_RETRY,
    T425_RL_FAILED
} t425_rl_state_t;

typedef struct {
    t425_rl_state_t state;
    uint8_t next_seq;
    uint8_t pending_seq;
    uint8_t retries;
    uint8_t max_retries;
    uint32_t deadline;
    uint32_t ack_timeout;
    uint32_t tx_ok;
    uint32_t rx_ok;
    uint32_t retries_total;
    uint32_t failures;
    uint32_t duplicates;
} t425_reliable_t;

void t425_rl_init(t425_reliable_t *r, uint8_t max_retries, uint32_t ack_timeout);
bool t425_rl_begin(t425_reliable_t *r, uint32_t now, uint8_t *seq);
bool t425_rl_ack(t425_reliable_t *r, uint8_t seq, uint32_t now);
void t425_rl_poll(t425_reliable_t *r, uint32_t now);
bool t425_rl_should_retry(const t425_reliable_t *r);
bool t425_rl_take_retry(t425_reliable_t *r, uint8_t *seq);
void t425_rl_tx_result(t425_reliable_t *r, bool ok);
