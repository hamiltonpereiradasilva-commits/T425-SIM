#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_FRAME_SOF 0xA5
#define T425_FRAME_MAX_PAYLOAD 240

typedef struct {
    uint8_t src,dst,type,seq,len;
    uint8_t payload[T425_FRAME_MAX_PAYLOAD];
    uint16_t crc;
} t425_frame_t;

uint16_t t425_crc16(const uint8_t *p,uint16_t n);
bool t425_frame_encode(const t425_frame_t *f,uint8_t *out,uint16_t cap,uint16_t *used);
bool t425_frame_decode(const uint8_t *in,uint16_t n,t425_frame_t *f);
