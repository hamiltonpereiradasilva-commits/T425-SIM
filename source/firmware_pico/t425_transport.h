#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_router.h"

#define T425_WIRE_MAGIC0 0x54u
#define T425_WIRE_MAGIC1 0x34u
#define T425_WIRE_VERSION 1u
#define T425_WIRE_MAX 80u

uint16_t t425_transport_encode(const t425_packet_t *p, uint8_t *out, uint16_t cap);
bool t425_transport_decode(const uint8_t *buf, uint16_t len, t425_packet_t *p);
