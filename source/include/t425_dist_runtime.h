#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_remote_rdv.h"

#define T425_DIST_MAX_NODES 4
#define T425_DIST_MAX_CHANS 16

typedef struct {
    uint8_t src;
    uint8_t dst;
    uint8_t chan;
    bool used;
} t425_route_t;

typedef struct {
    uint8_t node_id;
    t425_remote_rdv_t remote;
    t425_route_t routes[T425_DIST_MAX_CHANS];
    uint8_t route_count;
} t425_dist_runtime_t;

void t425_dist_init(t425_dist_runtime_t *d,uint8_t node_id);
bool t425_dist_bind(t425_dist_runtime_t *d,uint8_t chan,uint8_t src,uint8_t dst);
bool t425_dist_send(t425_dist_runtime_t *d,uint8_t chan,const uint8_t *data,uint8_t len,
                    t425_link_packet_t *packet);
bool t425_dist_receive(t425_dist_runtime_t *d,const t425_link_packet_t *packet,
                       t425_link_packet_t *reply);
