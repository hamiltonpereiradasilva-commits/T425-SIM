#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_LINK_SOF 0xA5u
#define T425_LINK_MAX_PAYLOAD 240u

typedef enum {
    T425_LDATA = 0x01,
    T425_LACK  = 0x02,
    T425_LPING = 0x03,
    T425_LPONG = 0x04,
    T425_LRESET= 0x05,
    T425_LHALT = 0x06,
    T425_LRUN,
    T425_LOUT  = 0x07
} t425_link_type_t;

typedef struct {
    uint8_t sof;
    uint8_t src;
    uint8_t dst;
    uint8_t type;
    uint8_t seq;
    uint8_t len;
    uint8_t payload[T425_LINK_MAX_PAYLOAD];
    uint8_t crc;
} t425_link_packet_t;

uint8_t t425_link_crc(const t425_link_packet_t *p);
bool t425_link_packet_encode(const t425_link_packet_t *p,uint8_t*out,uint16_t*len);
bool t425_link_packet_decode(const uint8_t*in,uint16_t len,t425_link_packet_t*p);
