#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_transport.h"
#include "t425_reliable_v2.h"

#define T425_ACK_DEST 0xFEu
#define T425_ACK_PAYLOAD 'A'

typedef bool (*t425_phy_tx_fn)(uint8_t link,const uint8_t*data,uint16_t len,uint32_t now,void*ctx);

typedef struct {
    t425_rl2_t rl;
    t425_phy_tx_fn tx;
    void *ctx;
    uint8_t link;
    uint8_t node_id;
    uint32_t rx_frames;
    uint32_t rx_bad_crc;
    uint32_t rx_ack;
    uint32_t rx_data;
} t425_link_endpoint_t;

void t425_link_endpoint_init(t425_link_endpoint_t*,uint8_t link,uint8_t node_id,
                             uint8_t max_retries,uint32_t timeout,
                             t425_phy_tx_fn,void*);
bool t425_link_send(t425_link_endpoint_t*,t425_packet_t*,uint32_t);
bool t425_link_receive(t425_link_endpoint_t*,const uint8_t*,uint16_t,uint32_t);
void t425_link_poll(t425_link_endpoint_t*,uint32_t);
