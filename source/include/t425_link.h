#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_frame.h"
#include "t425_reliable.h"

#define T425_LINK_RX_BUFFER 512
#define T425_LINK_TX_BUFFER 512

typedef struct {
    t425_reliable_t reliable;
    uint8_t rx_buf[T425_LINK_RX_BUFFER];
    uint16_t rx_len;
    uint8_t tx_buf[T425_LINK_TX_BUFFER];
    uint16_t tx_len;
    uint32_t frames_ok;
    uint32_t frames_bad;
} t425_link_stack_t;

void t425_link_stack_init(t425_link_stack_t *s);
bool t425_link_stack_receive(t425_link_stack_t *s, const uint8_t *raw,
                             uint16_t n, t425_frame_t *out);
bool t425_link_stack_prepare(t425_link_stack_t *s, uint8_t src, uint8_t dst,
                             const uint8_t *payload, uint8_t len,
                             uint16_t *wire_len);
