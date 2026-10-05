#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "t425_core_phy.h"

typedef struct
{
    int tx_calls;
    uint32_t tx_endpoint;
    uint16_t tx_frame;
    int tx_result;

    int rx_available_calls;
    uint32_t rx_available_endpoint;
    bool rx_available_result;

    int rx_get_calls;
    uint32_t rx_get_endpoint;
    uint32_t rx_get_value;
    int rx_get_result;
} mock_context_t;

static int mock_tx(
    void *context,
    uint32_t endpoint_id,
    uint16_t frame)
{
    mock_context_t *mock = context;

    mock->tx_calls++;
    mock->tx_endpoint = endpoint_id;
    mock->tx_frame = frame;

    return mock->tx_result;
}

static bool mock_rx_available(
    void *context,
    uint32_t endpoint_id)
{
    mock_context_t *mock = context;

    mock->rx_available_calls++;
    mock->rx_available_endpoint = endpoint_id;

    return mock->rx_available_result;
}

static int mock_rx_get(
    void *context,
    uint32_t endpoint_id,
    uint32_t *frame)
{
    mock_context_t *mock = context;

    mock->rx_get_calls++;
    mock->rx_get_endpoint = endpoint_id;

    if (frame != 0)
        *frame = mock->rx_get_value;

    return mock->rx_get_result;
}

static t425_core_phy_ops_t make_ops(mock_context_t *mock)
{
    t425_core_phy_ops_t ops;

    ops.tx_frame = mock_tx;
    ops.rx_available = mock_rx_available;
    ops.rx_get = mock_rx_get;
    ops.context = mock;
    ops.read_system_inputs = 0;
    ops.write_system_outputs = 0;
    ops.set_rx_admission = 0;
    ops.set_rx_admission_credit = 0;

    return ops;
}

static void test_init_rejects_invalid_inputs(void)
{
    t425_core_phy_t phy;
    mock_context_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);

    memset(&phy, 0, sizeof(phy));

    assert(!t425_core_phy_init(0, &ops, 4u));
    assert(!t425_core_phy_init(&phy, 0, 4u));
    assert(!t425_core_phy_init(&phy, &ops, 0u));
    assert(!t425_core_phy_init(
        &phy,
        &ops,
        T425_CORE_PHY_MAX_ENDPOINTS + 1u));

    ops.tx_frame = 0;
    assert(!t425_core_phy_init(&phy, &ops, 4u));

    ops = make_ops(&mock);
    ops.rx_available = 0;
    assert(!t425_core_phy_init(&phy, &ops, 4u));

    ops = make_ops(&mock);
    ops.rx_get = 0;
    assert(!t425_core_phy_init(&phy, &ops, 4u));
}

static void test_valid_init_copies_attachment(void)
{
    t425_core_phy_t phy = {0};
    mock_context_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);

    assert(t425_core_phy_init(&phy, &ops, 4u));
    assert(phy.attached);
    assert(phy.endpoint_count == 4u);
    assert(phy.ops.tx_frame == ops.tx_frame);
    assert(phy.ops.rx_available == ops.rx_available);
    assert(phy.ops.rx_get == ops.rx_get);
    assert(phy.ops.context == &mock);
}

static void test_tx(void)
{
    t425_core_phy_t phy = {0};
    mock_context_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);

    assert(t425_core_phy_init(
        &phy,
        &ops,
        T425_CORE_PHY_MAX_ENDPOINTS));

    mock.tx_result = -37;

    assert(t425_core_phy_tx(&phy, 0u, 0x12u) == -37);
    assert(mock.tx_calls == 1);
    assert(mock.tx_endpoint == 0u);
    assert(mock.tx_frame == 0x12u);

    mock.tx_calls = 0;
    assert(t425_core_phy_tx(&phy, 3u, 0xA5u) == -37);
    assert(mock.tx_calls == 1);
    assert(mock.tx_endpoint == 3u);
    assert(mock.tx_frame == 0xA5u);

    mock.tx_calls = 0;
    assert(t425_core_phy_tx(&phy, 4u, 0x22u) == -1);
    assert(mock.tx_calls == 0);

    assert(t425_core_phy_tx(&phy, 0u, 0x100u) == -1);
    assert(mock.tx_calls == 0);

    assert(t425_core_phy_tx(0, 0u, 0x22u) == -1);
    assert(mock.tx_calls == 0);

    phy.attached = false;
    assert(t425_core_phy_tx(&phy, 0u, 0x22u) == -1);
    assert(mock.tx_calls == 0);
}

static void test_rx_available(void)
{
    t425_core_phy_t phy = {0};
    mock_context_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);

    assert(t425_core_phy_init(
        &phy,
        &ops,
        T425_CORE_PHY_MAX_ENDPOINTS));

    mock.rx_available_result = true;

    assert(t425_core_phy_rx_available(&phy, 0u));
    assert(mock.rx_available_calls == 1);
    assert(mock.rx_available_endpoint == 0u);

    mock.rx_available_result = false;

    assert(!t425_core_phy_rx_available(&phy, 3u));
    assert(mock.rx_available_calls == 2);
    assert(mock.rx_available_endpoint == 3u);

    assert(!t425_core_phy_rx_available(&phy, 4u));
    assert(mock.rx_available_calls == 2);

    assert(!t425_core_phy_rx_available(0, 0u));
    assert(mock.rx_available_calls == 2);

    phy.attached = false;
    assert(!t425_core_phy_rx_available(&phy, 0u));
    assert(mock.rx_available_calls == 2);
}

static void test_rx_get(void)
{
    t425_core_phy_t phy = {0};
    mock_context_t mock = {0};
    t425_core_phy_ops_t ops = make_ops(&mock);
    uint32_t frame = 0u;

    assert(t425_core_phy_init(
        &phy,
        &ops,
        T425_CORE_PHY_MAX_ENDPOINTS));

    mock.rx_get_value = 0x12345678u;
    mock.rx_get_result = -23;

    assert(t425_core_phy_rx(&phy, 0u, &frame) == -23);
    assert(mock.rx_get_calls == 1);
    assert(mock.rx_get_endpoint == 0u);
    assert(frame == 0x12345678u);

    frame = 0u;
    assert(t425_core_phy_rx(&phy, 3u, &frame) == -23);
    assert(mock.rx_get_calls == 2);
    assert(mock.rx_get_endpoint == 3u);
    assert(frame == 0x12345678u);

    assert(t425_core_phy_rx(&phy, 4u, &frame) == -1);
    assert(mock.rx_get_calls == 2);

    assert(t425_core_phy_rx(&phy, 0u, 0) == -1);
    assert(mock.rx_get_calls == 2);

    assert(t425_core_phy_rx(0, 0u, &frame) == -1);
    assert(mock.rx_get_calls == 2);

    phy.attached = false;
    assert(t425_core_phy_rx(&phy, 0u, &frame) == -1);
    assert(mock.rx_get_calls == 2);
}

int main(void)
{
    test_init_rejects_invalid_inputs();
    test_valid_init_copies_attachment();
    test_tx();
    test_rx_available();
    test_rx_get();

    puts("F_PHY-63 contract behavior: PASS");
    return 0;
}
