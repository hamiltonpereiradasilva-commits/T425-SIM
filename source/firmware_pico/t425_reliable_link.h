#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_SYNC       0xA5u
#define T425_VERSION    1u
#define T425_TYPE_DATA  0x01u
#define T425_TYPE_ACK   0x02u
#define T425_MAX_PAYLOAD 32u
#define T425_HEADER_LEN  7u
#define T425_OVERHEAD    9u

typedef struct __attribute__((packed)) {
    uint8_t sync, version, type, src, dst, seq, len;
    uint8_t payload[T425_MAX_PAYLOAD];
    uint16_t crc16;
} t425_frame_t;

typedef struct {
    uint8_t buf[T425_OVERHEAD + T425_MAX_PAYLOAD];
    uint16_t pos, expected;
    uint32_t frames_ok;
    uint32_t crc_errors;
    uint32_t format_errors;
    uint32_t sync_losses;
} t425_parser_t;

typedef struct {
    uint8_t next_tx_seq;
    uint8_t waiting_seq;
    bool waiting_ack;
    uint8_t retries;
    uint32_t deadline_ms;
    uint32_t timeout_ms;
    uint8_t max_retries;
    uint8_t last_rx_seq;
    bool have_rx_seq;
    uint8_t pending_seq;
    bool pending_valid;
    uint32_t tx_frames, rx_frames, crc_errors, timeouts, retransmits, acked;
    uint32_t duplicate_data, out_of_order;
} t425_rl_t;

uint16_t t425_crc16_ccitt(const uint8_t*, uint16_t);
bool t425_frame_build(t425_frame_t*, uint8_t type, uint8_t src, uint8_t dst,
                      uint8_t seq, const uint8_t*, uint8_t);
bool t425_frame_check(const t425_frame_t*);
void t425_parser_init(t425_parser_t*);
bool t425_parser_push(t425_parser_t*, uint8_t, t425_frame_t*);
void t425_frame_corrupt_crc(t425_frame_t*);
void t425_rl_init(t425_rl_t*, uint32_t timeout_ms, uint8_t max_retries);
bool t425_rl_accept_ack(t425_rl_t*, uint8_t seq);
bool t425_rl_accept_data(t425_rl_t*, uint8_t seq, bool *duplicate);
void t425_rl_start_tx(t425_rl_t*, uint8_t seq, uint32_t now_ms);
bool t425_rl_timeout(t425_rl_t*, uint32_t now_ms);
void t425_rl_note_retransmit(t425_rl_t*, uint32_t now_ms);
void t425_rl_note_timeout(t425_rl_t*);

