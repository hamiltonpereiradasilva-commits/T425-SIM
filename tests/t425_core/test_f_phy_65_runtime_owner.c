#include "t425_runtime.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    unsigned tx_calls;
    unsigned rx_available_calls;
    unsigned rx_get_calls;
    uint32_t last_tx_endpoint;
    uint16_t last_tx_frame;
    bool accept_tx;
    bool rx_enabled;
    bool rx_pending;
    uint32_t configured_rx_endpoint;
    uint32_t last_queried_endpoint;
    uint32_t rx_frame;
} mock_t;

static int mock_tx(void *context, uint32_t endpoint_id, uint16_t frame)
{
    mock_t *mock = context;
    mock->tx_calls++;
    mock->last_tx_endpoint = endpoint_id;
    mock->last_tx_frame = frame;
    return mock->accept_tx ? 0 : -7;
}
static bool mock_rx_available(void *context, uint32_t endpoint_id)
{
    mock_t *mock = context;
    mock->rx_available_calls++;
    mock->last_queried_endpoint = endpoint_id;
    return mock->rx_enabled && mock->rx_pending &&
           endpoint_id == mock->configured_rx_endpoint;
}
static int mock_rx_get(void *context, uint32_t endpoint_id, uint32_t *frame)
{
    mock_t *mock = context;
    mock->rx_get_calls++;
    mock->last_queried_endpoint = endpoint_id;
    if (!mock->rx_enabled || !mock->rx_pending ||
        endpoint_id != mock->configured_rx_endpoint || frame == 0)
        return -9;
    *frame = mock->rx_frame;
    mock->rx_pending = false;
    return 0;
}
static t425_core_phy_ops_t make_ops(mock_t *mock)
{
    t425_core_phy_ops_t ops = {mock_tx, mock_rx_available, mock_rx_get, mock};
    return ops;
}
static void test_invalid_init(void)
{
    t425_runtime_t runtime = {0};
    mock_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);
    t425_core_phy_ops_t bad;
    assert(!t425_runtime_init(0, &ops));
    assert(!t425_runtime_init(&runtime, 0));
    bad = ops; bad.tx_frame = 0; assert(!t425_runtime_init(&runtime, &bad));
    bad = ops; bad.rx_available = 0; assert(!t425_runtime_init(&runtime, &bad));
    bad = ops; bad.rx_get = 0; assert(!t425_runtime_init(&runtime, &bad));
    assert(t425_runtime_core0_phy(&runtime) == 0);
    assert(t425_runtime_poll_core1(&runtime) == 0u);
}
static void test_runtime_binding(void)
{
    t425_runtime_t runtime = {0};
    mock_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);
    t425_core_phy_t *core0;
    uint32_t frame;
    mock.accept_tx = true;
    assert(t425_runtime_init(&runtime, &ops));
    assert(runtime.initialized);
    core0 = t425_runtime_core0_phy(&runtime);
    assert(core0 != 0);
    assert(core0 == t425_runtime_core0_phy(&runtime));
    assert(!t425_runtime_init(&runtime, &ops));
    assert(t425_core_phy_tx(core0, 2u, 0x5Au) == 0);
    assert(t425_runtime_poll_core1(&runtime) == 1u);
    assert(mock.tx_calls == 1u && mock.last_tx_endpoint == 2u &&
           mock.last_tx_frame == 0x5Au);
    mock.accept_tx = false;
    assert(t425_core_phy_tx(core0, 1u, 0x33u) == 0);
    assert(t425_runtime_poll_core1(&runtime) == 0u);
    assert(mock.tx_calls == 2u);
    mock.accept_tx = true;
    assert(t425_runtime_poll_core1(&runtime) == 1u);
    assert(mock.tx_calls == 3u && mock.last_tx_endpoint == 1u &&
           mock.last_tx_frame == 0x33u);
    mock.rx_enabled = true;
    mock.configured_rx_endpoint = 2u;
    mock.rx_pending = true;
    mock.rx_frame = 0xA5u;
    assert(!mock_rx_available(&mock, 0u));
    assert(!mock_rx_available(&mock, 1u));
    assert(mock_rx_available(&mock, 2u));
    assert(!mock_rx_available(&mock, 3u));
    assert(mock.configured_rx_endpoint == 2u);
    assert(mock_rx_get(&mock, 1u, &frame) == -9 && mock.rx_pending);
    assert(mock_rx_get(&mock, 2u, &frame) == 0 && frame == 0xA5u);
    mock.configured_rx_endpoint = 3u;
    mock.rx_frame = 0x77u;
    mock.rx_pending = true;
    assert(t425_runtime_poll_core1(&runtime) == 1u);
    assert(t425_core_phy_rx(core0, 3u, &frame) == 0 && frame == 0x77u);
    mock.configured_rx_endpoint = 0u;
    for (unsigned index = 0u; index < 8u; ++index) {
        mock.rx_frame = 0x40u + index;
        mock.rx_pending = true;
        assert(t425_runtime_poll_core1(&runtime) == 1u);
    }
    mock.rx_frame = 0xEEu;
    mock.rx_pending = true;
    assert(t425_runtime_poll_core1(&runtime) == 0u && mock.rx_pending);
    assert(t425_core_phy_rx(core0, 0u, &frame) == 0 && frame == 0x40u);
    assert(t425_runtime_poll_core1(&runtime) == 1u && !mock.rx_pending);
    assert(t425_core_phy_rx(core0, 0u, &frame) == 0 && frame == 0x41u);
}
int main(void)
{
    test_invalid_init();
    test_runtime_binding();
    puts("F_PHY-65 runtime ownership: PASS");
    return 0;
}
