#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "t425_core_phy.h"
#include "t425_core_phy_ipc.h"
#include "t425_core_phy_rp2040.h"

/* Deterministic physical-link double: the RP2040 adapter remains real. */
struct fake_pio_state { int id; }; struct fake_pio_state fake_pio0, fake_pio1;
typedef struct {
    uint32_t rx[8];
    unsigned rx_len;
    unsigned rx_pos;
    unsigned ack_count;
    unsigned tx_count;
    bool tx_accept;
} link_double_t;

static link_double_t *double_for(t425_link_t *link)
{
    return (link_double_t *)(uintptr_t)link->rx_buffer;
}

bool t425_link_send_frame(t425_link_t *link, uint16_t data)
{
    link_double_t *d = double_for(link);
    (void)data;
    if (!d->tx_accept)
        return false;
    ++d->tx_count;
    return true;
}

bool t425_link_send_ack(t425_link_t *link)
{
    link_double_t *d = double_for(link);
    ++d->ack_count;
    return true;
}

bool t425_link_rx_available(t425_link_t *link)
{
    link_double_t *d = double_for(link);
    return d->rx_pos < d->rx_len;
}

uint32_t t425_link_rx_get(t425_link_t *link)
{
    link_double_t *d = double_for(link);
    assert(d->rx_pos < d->rx_len);
    return d->rx[d->rx_pos++];
}

void t425_link_note_ack_received(t425_link_t *link) { (void)link; }

/* Unused lifecycle functions required by the adapter object. */
bool t425_link_init(t425_link_t *l, PIO p, uint a, uint b, uint c, uint d,
                    t425_link_speed_t s)
{ (void)l; (void)p; (void)a; (void)b; (void)c; (void)d; (void)s; return true; }
bool t425_link_dma_init(t425_link_t *l, uint32_t *b, uint32_t n)
{ (void)l; (void)b; (void)n; return true; }
void t425_link_dma_deinit(t425_link_t *l) { (void)l; }
void t425_link_start(t425_link_t *l) { (void)l; }
void t425_link_stop(t425_link_t *l) { (void)l; }

static uint32_t data_word(uint8_t byte)
{
    return (1u << 31) | (1u << 30) | ((uint32_t)byte << 22);
}

static void inject(link_double_t *d, uint8_t byte)
{
    assert(d->rx_len < 8u);
    d->rx[d->rx_len++] = data_word(byte);
}

static void publish_credit(t425_core_phy_t *core0, uint32_t endpoint,
                           uint32_t generation, bool enabled)
{
    assert(t425_core_phy_set_rx_admission_credit(
               core0, endpoint, generation, enabled) == 0);
}

static void poll(t425_core_phy_ipc_service_t *service)
{
    (void)t425_core_phy_ipc_service_poll(service);
}

static void expect_core_byte(t425_core_phy_t *core0, uint32_t endpoint,
                             uint8_t expected)
{
    uint32_t frame = 0u;
    assert(t425_core_phy_rx_available(core0, endpoint));
    assert(t425_core_phy_rx(core0, endpoint, &frame) == 0);
    assert(frame == expected);
}

int main(void)
{
    t425_core_phy_ipc_t ipc;
    t425_core_phy_ipc_service_t service;
    t425_core_phy_t core0;
    t425_core_phy_rp2040_t adapter;
    t425_core_phy_ops_t core0_ops = {0};
    t425_core_phy_ops_t adapter_ops = {0};
    t425_link_t links[4] = {0};
    t425_link_t *link_ptrs[4];
    t425_core_phy_route_t routes[4] = {{0u,0u},{1u,1u},{2u,2u},{3u,3u}};
    link_double_t doubles[4] = {0};

    for (unsigned i = 0u; i < 4u; ++i) {
        links[i].pio = (i < 2u) ? pio0 : pio1;
        links[i].sm_rx = (i & 1u) ? 3u : 1u;
        links[i].rx_buffer = (uint32_t *)(uintptr_t)&doubles[i];
        doubles[i].tx_accept = true;
        link_ptrs[i] = &links[i];
    }

    assert(t425_core_phy_ipc_init(&ipc));
    t425_core_phy_ipc_make_core0_ops(&ipc, &core0_ops);
    assert(t425_core_phy_init(&core0, &core0_ops, 4u));
    assert(t425_core_phy_rp2040_init(&adapter, link_ptrs, routes));
    t425_core_phy_rp2040_make_ops(&adapter, &adapter_ops);
    assert(t425_core_phy_ipc_service_init(&service, &ipc, &adapter_ops));

    /* A: one generation authorizes exactly one DATA and ACK. */
    publish_credit(&core0, 0u, 1u, true);
    poll(&service);
    inject(&doubles[0], 0xA1u);
    poll(&service);
    expect_core_byte(&core0, 0u, 0xA1u);
    assert(doubles[0].ack_count == 1u);
    assert(adapter.rx_consumed_generation[0] == 1u);

    /* Same generation, repeated propagation, cannot reopen credit. */
    poll(&service); poll(&service);
    inject(&doubles[0], 0xB2u);
    poll(&service); poll(&service);
    assert(!t425_core_phy_rx_available(&core0, 0u));
    assert(doubles[0].ack_count == 1u);
    assert(adapter.rx_data_pending[0]);

    /* B: fresh generation releases the already pending DATA exactly once. */
    publish_credit(&core0, 0u, 2u, true);
    poll(&service);
    expect_core_byte(&core0, 0u, 0xB2u);
    assert(doubles[0].ack_count == 2u);
    poll(&service);
    assert(doubles[0].ack_count == 2u);

    /* C/D: disable blocks old grant; fresh enable releases pending byte. */
    publish_credit(&core0, 0u, 3u, true);
    poll(&service);
    inject(&doubles[0], 0xC3u);
    publish_credit(&core0, 0u, 4u, false);
    poll(&service); poll(&service);
    assert(adapter.rx_data_pending[0]);
    assert(doubles[0].ack_count == 2u);
    publish_credit(&core0, 0u, 5u, true);
    poll(&service);
    expect_core_byte(&core0, 0u, 0xC3u);
    assert(doubles[0].ack_count == 3u);

    /* E: mailbox full preserves both DATA and the original credit. */
    publish_credit(&core0, 0u, 6u, true);
    poll(&service);
    for (unsigned i = 0u; i < T425_CORE_PHY_IPC_QUEUE_DEPTH; ++i) {
        t425_core_phy_ipc_byte_t msg = {(uint8_t)i};
        assert(queue_try_add(&ipc.core1_to_core0[0], &msg));
    }
    inject(&doubles[0], 0xD4u);
    poll(&service);
    assert(adapter.rx_data_pending[0]);
    assert(doubles[0].ack_count == 3u);
    for (unsigned i = 0u; i < T425_CORE_PHY_IPC_QUEUE_DEPTH; ++i) {
        t425_core_phy_ipc_byte_t msg;
        assert(queue_try_remove(&ipc.core1_to_core0[0], &msg));
    }
    /* Destination capacity is restored; the original credit remains usable. */
    poll(&service);
    expect_core_byte(&core0, 0u, 0xD4u);
    assert(doubles[0].ack_count == 4u);

    /* G: LOAD N credit chain and N+1 retention across interleavings. */
    {
        const uint8_t payload[3] = {0x31u, 0x32u, 0x33u};
        for (unsigned i = 0u; i < 3u; ++i) {
            publish_credit(&core0, 3u, 20u + i, true);
            if ((i & 1u) == 0u) poll(&service);
            inject(&doubles[3], payload[i]);
            poll(&service);
            expect_core_byte(&core0, 3u, payload[i]);
        }
        inject(&doubles[3], 0x3Eu);
        poll(&service); poll(&service);
        assert(adapter.rx_data_pending[3]);
        assert(doubles[3].ack_count == 3u);
    }
    /* F: independent endpoint grants and wrap-safe generations. */
    adapter.rx_grant_generation[1] = 0xFFFFFFFDu; adapter.rx_consumed_generation[1] = 0xFFFFFFFDu;
    adapter.rx_grant_generation[2] = 0xFFFFFFFEu; adapter.rx_consumed_generation[2] = 0xFFFFFFFEu;
    publish_credit(&core0, 1u, 0xFFFFFFFEu, true);
    publish_credit(&core0, 2u, 0xFFFFFFFFu, true);
    poll(&service);
    inject(&doubles[1], 0xE1u); inject(&doubles[2], 0xE2u);
    poll(&service);
    expect_core_byte(&core0, 1u, 0xE1u);
    expect_core_byte(&core0, 2u, 0xE2u);
    assert(doubles[1].ack_count == 1u && doubles[2].ack_count == 1u);
    publish_credit(&core0, 1u, 1u, true);
    poll(&service);
    inject(&doubles[1], 0xE3u);
    poll(&service);
    expect_core_byte(&core0, 1u, 0xE3u);
    assert(doubles[1].ack_count == 2u);

    puts("F_PHY-71D end-to-end credit race: PASS");
    return 0;
}
