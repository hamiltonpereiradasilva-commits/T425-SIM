#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_packet.h"

typedef struct {
    t425_link_packet_t packet;
    uint8_t raw[7 + T425_LINK_MAX_PAYLOAD];
    uint16_t used;
    bool active;
} t425_link_stream_t;

void t425_link_stream_init(t425_link_stream_t *s);
bool t425_link_stream_push(t425_link_stream_t *s, uint8_t byte,
                           t425_link_packet_t *out);
