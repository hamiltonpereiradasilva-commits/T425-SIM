#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_packet.h"

#define T425_OCCAM_CHAN_CAPACITY 16
#define T425_OCCAM_MAX_VALUE 240

typedef struct {
    uint8_t src_node;
    uint8_t dst_node;
    uint8_t chan_id;
    uint8_t seq;
    bool remote;
} t425_occam_chan_t;

typedef struct {
    uint8_t chan_id;
    uint8_t value[T425_OCCAM_MAX_VALUE];
    uint8_t len;
} t425_occam_message_t;

void t425_occam_chan_init(t425_occam_chan_t *c,uint8_t src,uint8_t dst,uint8_t id);
bool t425_occam_encode_out(const t425_occam_chan_t *c,
                           const uint8_t *data,uint8_t len,
                           t425_link_packet_t *p);
bool t425_occam_decode_in(const t425_occam_chan_t *c,
                          const t425_link_packet_t *p,
                          t425_occam_message_t *m);
