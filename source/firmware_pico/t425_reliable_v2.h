#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_transport.h"

#define T425_RL2_MAX_RETRIES 5u
#define T425_RL2_TIMEOUT 100u

typedef enum {
 T425_RL2_IDLE=0, T425_RL2_WAIT_ACK, T425_RL2_RETRY, T425_RL2_FAILED
} t425_rl2_state_t;

typedef struct {
 t425_rl2_state_t state;
 uint8_t next_seq, pending_seq, retries, max_retries;
 uint32_t tx_time, deadline, ack_timeout;
 uint32_t tx_ok, ack_ok, retries_total, failures, duplicates;
 t425_packet_t pending_packet;
 bool pending_valid;
} t425_rl2_t;

void t425_rl2_init(t425_rl2_t*,uint8_t,uint32_t);
bool t425_rl2_begin(t425_rl2_t*,t425_packet_t*,uint32_t,uint8_t*);
bool t425_rl2_ack(t425_rl2_t*,uint8_t,uint32_t);
void t425_rl2_poll(t425_rl2_t*,uint32_t);
bool t425_rl2_take_retry(t425_rl2_t*,t425_packet_t*,uint8_t*);
void t425_rl2_tx_result(t425_rl2_t*,bool);
