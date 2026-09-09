#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_link_packet.h"
#include "t425_rendezvous.h"

#define T425_RDV_REQ  0x20
#define T425_RDV_ACK  0x21
#define T425_RDV_CANCEL 0x22

typedef struct {
    uint8_t node_id;
    uint8_t next_seq;
    bool pending;
    uint8_t pending_seq;
    uint8_t pending_chan;
    uint8_t pending_dst;
} t425_remote_rdv_t;

void t425_remote_rdv_init(t425_remote_rdv_t *r,uint8_t node_id);
bool t425_remote_rdv_make_out(t425_remote_rdv_t *r,uint8_t dst,uint8_t chan,
                              const uint8_t *data,uint8_t len,t425_link_packet_t *p);
bool t425_remote_rdv_make_ack(t425_remote_rdv_t *r,uint8_t dst,uint8_t chan,
                              uint8_t seq,t425_link_packet_t *p);
bool t425_remote_rdv_handle(t425_remote_rdv_t *r,const t425_link_packet_t *in,
                            t425_link_packet_t *reply);
