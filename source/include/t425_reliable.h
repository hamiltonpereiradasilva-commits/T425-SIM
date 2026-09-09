#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_frame.h"

#define T425_RETRY_MAX 3
#define T425_ACK_TYPE 0x06
#define T425_NACK_TYPE 0x15
#define T425_DATA_TYPE 0x20

typedef struct {
    uint8_t next_seq;
    uint8_t last_rx_seq;
    uint8_t retries;
    uint32_t tx_count, rx_count, ack_count, retry_count, crc_errors, duplicates, timeouts;
} t425_reliable_t;

void t425_reliable_init(t425_reliable_t *r);
bool t425_reliable_prepare(t425_reliable_t *r, t425_frame_t *f,
                           uint8_t src, uint8_t dst,
                           const uint8_t *data, uint8_t len);
bool t425_reliable_is_ack(const t425_frame_t *f, uint8_t seq);
bool t425_reliable_is_duplicate(t425_reliable_t *r, const t425_frame_t *f);
void t425_reliable_note_tx(t425_reliable_t *r);
void t425_reliable_note_ack(t425_reliable_t *r);
void t425_reliable_note_retry(t425_reliable_t *r);
void t425_reliable_note_timeout(t425_reliable_t *r);
void t425_reliable_note_rx(t425_reliable_t *r);
void t425_reliable_note_crc_error(t425_reliable_t *r);
