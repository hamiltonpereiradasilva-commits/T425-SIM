#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_FWD_VERSION 1u
#define T425_FWD_MAX_HOPS 16u

typedef struct __attribute__((packed)) {
    uint8_t version;
    uint8_t src;
    uint8_t dst;
    uint8_t final_dst;
    uint8_t hop_count;
    uint8_t ttl;
    uint8_t payload_len;
} t425_forward_hdr_t;

bool t425_forward_build(t425_forward_hdr_t *h, uint8_t src, uint8_t dst,
                        uint8_t final_dst, uint8_t ttl, uint8_t payload_len);
bool t425_forward_forwardable(const t425_forward_hdr_t *h);
void t425_forward_advance(t425_forward_hdr_t *h, uint8_t next_hop);
