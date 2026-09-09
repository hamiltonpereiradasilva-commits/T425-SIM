#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_stream.h"
typedef struct {
    uint8_t node_id;
    uint8_t next_seq;
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t crc_errors;
    t425_link_stream_t stream[2];
} t425_link_node_t;
void t425_link_node_init(t425_link_node_t*n,uint8_t id);
bool t425_link_node_rx_byte(t425_link_node_t*n,uint8_t link,uint8_t byte,t425_link_packet_t*out);
