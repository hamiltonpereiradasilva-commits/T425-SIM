#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_frame.h"
#define T425_ACK_FRAME_TYPE 0x06
bool t425_make_ack(uint8_t src,uint8_t dst,uint8_t seq,t425_frame_t *f);
