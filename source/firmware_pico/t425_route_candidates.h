#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_MAX_ROUTE_CANDIDATES 4

typedef struct {
    uint8_t next_hop;
    uint8_t metric;
    bool valid;
} t425_route_candidate_t;

typedef struct {
    uint8_t destination;
    t425_route_candidate_t candidates[T425_MAX_ROUTE_CANDIDATES];
    uint8_t count;
} t425_route_set_t;

void t425_route_set_init(t425_route_set_t *r, uint8_t destination);
bool t425_route_add(t425_route_set_t *r, uint8_t next_hop, uint8_t metric);
bool t425_route_select(const t425_route_set_t *r,
                       const bool *peer_up, uint8_t peer_count,
                       uint8_t *next_hop);
