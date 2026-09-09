#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_frame.h"

typedef enum {
    T425_STREAM_WAIT_SOF = 0,
    T425_STREAM_HEADER,
    T425_STREAM_PAYLOAD,
    T425_STREAM_CRC
} t425_stream_state_t;

typedef struct {
    t425_stream_state_t state;
    t425_frame_t frame;
    uint8_t header[5];
    uint8_t header_pos;
    uint16_t payload_pos;
    uint8_t crc_bytes[2];
    uint8_t crc_pos;
    uint32_t bytes_seen;
    uint32_t frames_ok;
    uint32_t frames_bad;
} t425_stream_parser_t;

void t425_stream_init(t425_stream_parser_t *p);
bool t425_stream_push(t425_stream_parser_t *p, uint8_t byte,
                      t425_frame_t *complete);
