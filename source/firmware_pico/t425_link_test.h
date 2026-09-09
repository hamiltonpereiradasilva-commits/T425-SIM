#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_TEST_MAGIC0 0x54u
#define T425_TEST_MAGIC1 0x34u
#define T425_TEST_VERSION 1u
#define T425_TEST_PING 0x01u
#define T425_TEST_ACK  0x02u
#define T425_TEST_MAX_PAYLOAD 32u

typedef struct {
    uint8_t type;
    uint8_t seq;
    uint8_t len;
    uint8_t payload[T425_TEST_MAX_PAYLOAD];
} t425_test_frame_t;

uint16_t t425_test_encode(const t425_test_frame_t *f, uint8_t *out, uint16_t cap);
bool t425_test_decode_byte(uint8_t b, t425_test_frame_t *out);
void t425_test_parser_reset(void);
