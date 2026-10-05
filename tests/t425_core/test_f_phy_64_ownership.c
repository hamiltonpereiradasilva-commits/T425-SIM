#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core_phy_bootstrap.h"

typedef struct
{
    bool tx_accept;
    int tx_calls;
    uint32_t tx_endpoint;
    uint16_t tx_frame;

    bool rx_available;
    bool rx_frame_pending;
    uint32_t configured_rx_endpoint;
    uint32_t queried_rx_endpoint;
    int rx_available_calls;

    int rx_get_calls;
    uint32_t rx_get_endpoint;
    uint32_t rx_value;
} mock_phy_t;

static int mock_tx(
    void *context,
    uint32_t endpoint_id,
    uint16_t frame)
{
    mock_phy_t *mock = context;

    mock->tx_calls++;
    mock->tx_endpoint = endpoint_id;
    mock->tx_frame = frame;
    return mock->tx_accept ? 0 : -7;
}

static bool mock_rx_available(
    void *context,
    uint32_t endpoint_id)
{
    mock_phy_t *mock = context;

    mock->rx_available_calls++;
    mock->queried_rx_endpoint = endpoint_id;
    return mock->rx_available &&
           mock->rx_frame_pending &&
           endpoint_id == mock->configured_rx_endpoint;
}

static int mock_rx_get(
    void *context,
    uint32_t endpoint_id,
    uint32_t *frame)
{
    mock_phy_t *mock = context;

    mock->rx_get_calls++;
    mock->rx_get_endpoint = endpoint_id;
    if (!mock->rx_available ||
        !mock->rx_frame_pending ||
        endpoint_id != mock->configured_rx_endpoint ||
        frame == 0)
        return -8;

    *frame = mock->rx_value;
    mock->rx_frame_pending = false;
    return 0;
}

static t425_core_phy_ops_t make_ops(mock_phy_t *mock)
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

static void inject_rx(
    t425_core_phy_bootstrap_t *bootstrap,
    uint32_t endpoint_id,
    uint8_t byte)
{
    t425_core_phy_ipc_byte_t message;

    message.byte = byte;
    assert(queue_try_add(
        &bootstrap->ipc.core1_to_core0[endpoint_id],
        &message));
}

static void test_invalid_initialization(mock_phy_t *mock)
{
    t425_core_phy_bootstrap_t bootstrap = {0};
    t425_core_phy_ops_t ops = make_ops(mock);

    assert(!t425_core_phy_bootstrap_init(0, &ops));
    assert(!t425_core_phy_bootstrap_init(&bootstrap, 0));

    ops.tx_frame = 0;
    assert(!t425_core_phy_bootstrap_init(&bootstrap, &ops));

    ops = make_ops(mock);
    ops.rx_available = 0;
    assert(!t425_core_phy_bootstrap_init(&bootstrap, &ops));

    ops = make_ops(mock);
    ops.rx_get = 0;
    assert(!t425_core_phy_bootstrap_init(&bootstrap, &ops));
}

static void test_valid_initialization(
    t425_core_phy_bootstrap_t *bootstrap,
    mock_phy_t *mock)
{
    t425_core_phy_ops_t ops = make_ops(mock);
    t425_core_phy_t *core0;

    assert(t425_core_phy_bootstrap_init(bootstrap, &ops));
    assert(bootstrap->initialized);
    core0 = t425_core_phy_bootstrap_core0(bootstrap);
    assert(core0 == &bootstrap->core0_phy);
    assert(core0->attached);
    assert(core0->ops.context == &bootstrap->ipc);
    assert(bootstrap->core1_service.ipc == &bootstrap->ipc);
    assert(!t425_core_phy_bootstrap_init(bootstrap, &ops));
}

static void test_rx_endpoint_identity(mock_phy_t *mock)
{
    uint32_t frame = 0u;

    mock->rx_available = true;
    mock->rx_frame_pending = true;
    mock->configured_rx_endpoint = 2u;
    mock->queried_rx_endpoint = 99u;
    mock->rx_value = 0xD2u;

    assert(!mock_rx_available(mock, 0u));
    assert(mock->configured_rx_endpoint == 2u);
    assert(!mock_rx_available(mock, 1u));
    assert(mock->configured_rx_endpoint == 2u);
    assert(mock_rx_available(mock, 2u));
    assert(mock->configured_rx_endpoint == 2u);
    assert(!mock_rx_available(mock, 3u));
    assert(mock->configured_rx_endpoint == 2u);

    assert(mock_rx_get(mock, 1u, &frame) == -8);
    assert(mock->rx_frame_pending);
    assert(mock_rx_get(mock, 2u, &frame) == 0);
    assert(frame == 0xD2u);
    assert(!mock->rx_frame_pending);
    assert(mock_rx_get(mock, 2u, &frame) == -8);
}

static void test_core0_tx(
    t425_core_phy_bootstrap_t *bootstrap)
{
    t425_core_phy_ipc_byte_t message;
    t425_core_phy_t *core0 =
        t425_core_phy_bootstrap_core0(bootstrap);

    assert(t425_core_phy_tx(core0, 2u, 0x5Au) == 0);
    assert(queue_try_remove(
        &bootstrap->ipc.core0_to_core1[2u],
        &message));
    assert(message.byte == 0x5Au);
}

static void test_core0_rx(
    t425_core_phy_bootstrap_t *bootstrap)
{
    uint32_t frame = 0u;
    t425_core_phy_t *core0 =
        t425_core_phy_bootstrap_core0(bootstrap);

    inject_rx(bootstrap, 1u, 0xA6u);
    assert(t425_core_phy_rx_available(core0, 1u));
    assert(t425_core_phy_rx(core0, 1u, &frame) == 0);
    assert(frame == 0xA6u);
}

static void test_core1_tx(
    t425_core_phy_bootstrap_t *bootstrap,
    mock_phy_t *mock)
{
    t425_core_phy_t *core0 =
        t425_core_phy_bootstrap_core0(bootstrap);

    mock->tx_accept = true;
    assert(t425_core_phy_tx(core0, 3u, 0x31u) == 0);
    assert(t425_core_phy_bootstrap_poll_core1(bootstrap) == 1u);
    assert(mock->tx_endpoint == 3u);
    assert(mock->tx_frame == 0x31u);
}

static void test_core1_tx_backpressure(
    t425_core_phy_bootstrap_t *bootstrap,
    mock_phy_t *mock)
{
    t425_core_phy_ipc_byte_t message;
    t425_core_phy_t *core0 =
        t425_core_phy_bootstrap_core0(bootstrap);

    mock->tx_accept = false;
    assert(t425_core_phy_tx(core0, 0u, 0x44u) == 0);
    assert(t425_core_phy_bootstrap_poll_core1(bootstrap) == 0u);
    assert(queue_try_peek(
        &bootstrap->ipc.core0_to_core1[0u],
        &message));
    assert(message.byte == 0x44u);

    mock->tx_accept = true;
    assert(t425_core_phy_bootstrap_poll_core1(bootstrap) == 1u);
    assert(!queue_try_peek(
        &bootstrap->ipc.core0_to_core1[0u],
        &message));
}

static void test_core1_rx(
    t425_core_phy_bootstrap_t *bootstrap,
    mock_phy_t *mock)
{
    uint32_t frame = 0u;
    t425_core_phy_t *core0 =
        t425_core_phy_bootstrap_core0(bootstrap);

    mock->rx_available = true;
    mock->rx_frame_pending = true;
    mock->configured_rx_endpoint = 3u;
    mock->rx_value = 0xB7u;
    mock->rx_get_calls = 0;

    assert(t425_core_phy_bootstrap_poll_core1(bootstrap) == 1u);
    assert(mock->rx_get_calls == 1);
    assert(mock->rx_get_endpoint == 3u);
    assert(t425_core_phy_rx(core0, 3u, &frame) == 0);
    assert(frame == 0xB7u);
    mock->rx_available = false;
}

static void test_rx_full_preservation(
    t425_core_phy_bootstrap_t *bootstrap,
    mock_phy_t *mock)
{
    t425_core_phy_ipc_byte_t message;
    uint32_t frame = 0u;
    uint32_t i;
    t425_core_phy_t *core0 =
        t425_core_phy_bootstrap_core0(bootstrap);

    message.byte = 0x11u;
    for (i = 0u; i < T425_CORE_PHY_IPC_QUEUE_DEPTH; ++i)
        assert(queue_try_add(
            &bootstrap->ipc.core1_to_core0[0u],
            &message));

    mock->rx_available = true;
    mock->rx_frame_pending = true;
    mock->configured_rx_endpoint = 0u;
    mock->rx_value = 0xC8u;
    mock->rx_get_calls = 0;

    assert(t425_core_phy_bootstrap_poll_core1(bootstrap) == 0u);
    assert(mock->rx_get_calls == 0);
    assert(mock->rx_frame_pending);

    assert(queue_try_remove(
        &bootstrap->ipc.core1_to_core0[0u],
        &message));
    assert(t425_core_phy_bootstrap_poll_core1(bootstrap) == 1u);
    assert(mock->rx_get_calls == 1);

    for (i = 0u; i < T425_CORE_PHY_IPC_QUEUE_DEPTH - 1u; ++i)
        assert(queue_try_remove(
            &bootstrap->ipc.core1_to_core0[0u],
            &message));

    assert(t425_core_phy_rx(core0, 0u, &frame) == 0);
    assert(frame == 0xC8u);
    assert(!mock->rx_frame_pending);
    mock->rx_available = false;
}

static void test_uninitialized_poll(void)
{
    t425_core_phy_bootstrap_t bootstrap = {0};

    assert(t425_core_phy_bootstrap_core0(&bootstrap) == 0);
    assert(t425_core_phy_bootstrap_poll_core1(&bootstrap) == 0u);
}

int main(void)
{
    t425_core_phy_bootstrap_t bootstrap = {0};
    mock_phy_t mock = {0};

    test_invalid_initialization(&mock);
    test_valid_initialization(&bootstrap, &mock);
    test_rx_endpoint_identity(&mock);
    test_core0_tx(&bootstrap);
    test_core0_rx(&bootstrap);
    test_core1_tx(&bootstrap, &mock);
    test_core1_tx_backpressure(&bootstrap, &mock);
    test_core1_rx(&bootstrap, &mock);
    test_rx_full_preservation(&bootstrap, &mock);
    test_uninitialized_poll();

    puts("F_PHY-64 ownership binding: PASS");
    return 0;
}
