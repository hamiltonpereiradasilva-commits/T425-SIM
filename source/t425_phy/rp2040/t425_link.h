#ifndef T425_LINK_H
#define T425_LINK_H

#include <stdint.h>
#include <stdbool.h>

#include "hardware/pio.h"

typedef enum {
    T425_LINK_SPEED_5M  = 5,
    T425_LINK_SPEED_10M = 10,
    T425_LINK_SPEED_20M = 20
} t425_link_speed_t;

typedef struct {
    PIO pio;
    uint sm_tx;
    uint sm_rx;
    int tx_dma;
    int rx_dma;
    uint32_t *rx_buffer;
    uint32_t rx_buffer_words;
    uint32_t rx_dma_completed_base;
    uint32_t rx_dma_consumed;
    uint8_t rx_dma_ring_bits;
    bool rx_dma_started;
    bool rx_dma_full;
    uint8_t tx_word;
    uint32_t tx_dma_word;
    volatile bool tx_waiting_ack;
    uint tx_pin;
    uint rx_pin;
    t425_link_speed_t speed;
    uint tx_offset;
    uint rx_offset;
} t425_link_t;

bool t425_link_init(
    t425_link_t *link,
    PIO pio,
    uint sm_tx,
    uint sm_rx,
    uint tx_pin,
    uint rx_pin,
    t425_link_speed_t speed
);

bool t425_link_dma_init(
    t425_link_t *link,
    uint32_t *rx_buffer,
    uint32_t rx_buffer_words
);

void t425_link_dma_deinit(t425_link_t *link);

void t425_link_start(t425_link_t *link);
void t425_link_stop(t425_link_t *link);

/* Returns true only when local PHY submission is accepted/started. */
bool t425_link_send_frame(t425_link_t *link, uint16_t data);

/* Returns true only when local PHY submission is accepted/started. */
bool t425_link_send_ack(t425_link_t *link);

/* Clears only the local tx_waiting_ack state. */
void t425_link_note_ack_received(t425_link_t *link);

bool t425_link_rx_available(t425_link_t *link);
uint32_t t425_link_rx_get(t425_link_t *link);

#endif
