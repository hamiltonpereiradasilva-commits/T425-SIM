#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_FRAME_SYNC 0xA5u
#define T425_FRAME_VERSION 1u

typedef struct __attribute__((packed)) {
    uint8_t  sync;
    uint8_t  version;
    uint8_t  src;
    uint8_t  dst;
    uint8_t  seq;
    uint8_t  len;
    uint8_t  payload[32];
    uint16_t crc16;
} t425_frame_t;

uint16_t t425_crc16_ccitt(const uint8_t *data, uint16_t len);
bool t425_frame_build(t425_frame_t *f, uint8_t src, uint8_t dst,
                      uint8_t seq, const uint8_t *payload, uint8_t len);
bool t425_frame_check(const t425_frame_t *f);
