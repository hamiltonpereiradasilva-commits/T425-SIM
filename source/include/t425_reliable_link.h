#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_packet.h"

#define T425_RETRY_MAX 3
#define T425_ACK_TIMEOUT_TICKS 100

typedef struct {
    uint8_t node_id;
    uint8_t next_seq;
    uint8_t pending_seq;
    uint8_t pending_dst;
    uint8_t retries;
    bool waiting_ack;
    uint32_t tx_packets, rx_packets, retransmissions, timeouts, duplicates;
} t425_reliable_link_t;

void t425_reliable_init(t425_reliable_link_t *l,uint8_t node_id);
bool t425_reliable_make_data(t425_reliable_link_t *l,uint8_t dst,
                             const uint8_t *data,uint8_t len,
                             t425_link_packet_t *p);
bool t425_reliable_make_ack(t425_reliable_link_t *l,uint8_t dst,uint8_t seq,
                            t425_link_packet_t *p);
bool t425_reliable_on_packet(t425_reliable_link_t *l,
                             const t425_link_packet_t *in,
                             t425_link_packet_t *reply);
bool t425_reliable_timeout(t425_reliable_link_t *l);
