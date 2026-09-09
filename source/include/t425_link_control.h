#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_packet.h"

typedef struct {
    uint8_t node_id;
    uint8_t seq;
    uint32_t tx;
    uint32_t rx;
    uint32_t crc_errors;
    uint32_t timeouts;
} t425_link_control_t;

void t425_link_control_init(t425_link_control_t *c,uint8_t node_id);
bool t425_make_ping(t425_link_control_t *c,uint8_t dst,t425_link_packet_t *p);
bool t425_make_pong(t425_link_control_t *c,uint8_t dst,uint8_t seq,t425_link_packet_t *p);
bool t425_make_data(t425_link_control_t *c,uint8_t dst,const uint8_t *d,uint8_t n,t425_link_packet_t *p);
bool t425_handle_packet(t425_link_control_t *c,const t425_link_packet_t *in,
                        t425_link_packet_t *reply);
