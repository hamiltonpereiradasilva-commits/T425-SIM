#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "t425_link.h"

extern PIO test_pio;
extern int test_dma_fail_claim_number;
extern int test_dma_claim_count;
extern int test_dma_abort_count;
extern int test_dma_unclaim_count;

static void reset_dma_shim(void)
{
    test_dma_fail_claim_number = -1;
    test_dma_claim_count = 0;
    test_dma_abort_count = 0;
    test_dma_unclaim_count = 0;
}

static void test_init_state(void)
{
    t425_link_t link;
    memset(&link, 0xA5, sizeof(link));

    assert(t425_link_init(&link, test_pio, 0u, 1u, 0u, 1u,
                          T425_LINK_SPEED_10M));
    assert(link.tx_dma == -1);
    assert(link.rx_dma == -1);
    assert(link.tx_dma_word == 0u);
    assert(!link.tx_waiting_ack);
    assert(!link.rx_dma_started);
    assert(!link.rx_dma_full);
    assert(link.rx_dma_completed_base == 0u);
    assert(link.rx_dma_consumed == 0u);
    assert(link.rx_dma_ring_bits == 0u);
    assert(link.rx_buffer == 0);
    assert(link.rx_buffer_words == 0u);
}

static void test_success_and_deinit(void)
{
    t425_link_t link;
    uint32_t rx_buffer[4] = {0xA5A5A5A5u, 0u, 0u, 0u};

    reset_dma_shim();
    assert(t425_link_init(&link, test_pio, 0u, 1u, 0u, 1u,
                          T425_LINK_SPEED_10M));
    assert(t425_link_dma_init(&link, rx_buffer, 4u));
    assert(link.tx_dma >= 0);
    assert(link.rx_dma >= 0);
    assert(link.rx_buffer == rx_buffer);
    assert(link.rx_buffer_words == 4u);
    assert(t425_link_send_frame(&link, 0x55u));
    assert(t425_link_send_ack(&link));

    link.tx_waiting_ack = true;
    link.tx_word = 0x77u;
    t425_link_note_ack_received(&link);
    assert(!link.tx_waiting_ack);
    assert(link.tx_word == 0x77u);

    t425_link_dma_deinit(&link);
    assert(link.tx_dma == -1);
    assert(link.rx_dma == -1);
    assert(link.rx_buffer == 0);
    assert(link.rx_buffer_words == 0u);
    assert(!link.rx_dma_started);
    assert(!link.rx_dma_full);
    assert(link.rx_dma_completed_base == 0u);
    assert(link.rx_dma_consumed == 0u);
    assert(link.rx_dma_ring_bits == 0u);
    assert(test_dma_abort_count == 2);
    assert(test_dma_unclaim_count == 2);
    assert(rx_buffer[0] == 0xA5A5A5A5u);

    t425_link_dma_deinit(&link);
    assert(test_dma_abort_count == 2);
    assert(test_dma_unclaim_count == 2);
}

static void test_partial_claim_rollback(void)
{
    t425_link_t link;
    uint32_t rx_buffer[2] = {0u, 0u};

    reset_dma_shim();
    test_dma_fail_claim_number = 2;
    assert(t425_link_init(&link, test_pio, 0u, 1u, 0u, 1u,
                          T425_LINK_SPEED_10M));
    assert(!t425_link_dma_init(&link, rx_buffer, 2u));
    assert(link.tx_dma == -1);
    assert(link.rx_dma == -1);
    assert(link.rx_buffer == 0);
    assert(link.rx_buffer_words == 0u);
    assert(test_dma_abort_count == 1);
    assert(test_dma_unclaim_count == 1);
}

static void test_local_acceptance_rejection(void)
{
    t425_link_t link;
    uint32_t rx_buffer[1] = {0u};

    reset_dma_shim();
    assert(t425_link_init(&link, test_pio, 0u, 1u, 0u, 1u,
                          T425_LINK_SPEED_10M));
    assert(!t425_link_send_frame(&link, 0x55u));
    assert(!t425_link_send_ack(&link));
    assert(t425_link_dma_init(&link, rx_buffer, 1u));
    assert(t425_link_send_frame(&link, 0x55u));
    assert(t425_link_send_ack(&link));
}

int main(void)
{
    test_init_state();
    test_success_and_deinit();
    test_partial_claim_rollback();
    test_local_acceptance_rejection();
    puts("F_PHY-67 link API/lifecycle: PASS");
    return 0;
}
