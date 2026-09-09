#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_node_config.h"
#include "t425_stream.h"
#include "t425_ack.h"

typedef enum {
    T425_RT_BOOT=0,
    T425_RT_READY,
    T425_RT_RUNNING,
    T425_RT_ERROR
} t425_runtime_state_t;

typedef struct {
    t425_runtime_state_t state;
    t425_node_config_t config;
    t425_stream_parser_t rx_parser[T425_MAX_LINKS];
    t425_ack_ctrl_t ack[T425_MAX_LINKS];
    uint32_t ticks;
    uint32_t frames_rx;
    uint32_t frames_tx;
    uint32_t frames_bad;
} t425_runtime_t;

bool t425_runtime_init(t425_runtime_t *rt,
                       const t425_node_config_t *cfg,
                       uint8_t max_retries,
                       uint32_t ack_timeout_ticks);
void t425_runtime_tick(t425_runtime_t *rt);
bool t425_runtime_rx_byte(t425_runtime_t *rt, uint8_t local_link,
                          uint8_t byte, t425_frame_t *complete);
bool t425_runtime_ack_received(t425_runtime_t *rt, uint8_t local_link,
                               uint8_t seq);
