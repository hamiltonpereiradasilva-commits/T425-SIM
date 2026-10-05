#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

typedef struct
{
    unsigned tx_calls;
    uint32_t tx_endpoint;
    uint16_t tx_frame;
    bool rx_pending[4];
    uint32_t rx_frame[4];
    unsigned rx_available_calls;
    unsigned rx_get_calls;
} mock_t;

static int mock_tx(void *context, uint32_t endpoint, uint16_t frame)
{
    mock_t *mock = context;
    mock->tx_calls++;
    mock->tx_endpoint = endpoint;
    mock->tx_frame = frame;
    return 0;
}

static bool mock_rx_available(void *context, uint32_t endpoint)
{
    mock_t *mock = context;
    mock->rx_available_calls++;
    return endpoint < 4u && mock->rx_pending[endpoint];
}

static int mock_rx_get(void *context, uint32_t endpoint, uint32_t *frame)
{
    mock_t *mock = context;
    mock->rx_get_calls++;
    if (endpoint >= 4u || frame == 0 || !mock->rx_pending[endpoint])
        return -1;
    *frame = mock->rx_frame[endpoint];
    mock->rx_pending[endpoint] = false;
    return 0;
}

static t425_core_phy_ops_t mock_ops(mock_t *mock)
{
    t425_core_phy_ops_t ops = {
        mock_tx, mock_rx_available, mock_rx_get, mock, 0, 0
    };
    return ops;
}

static void test_tx_mapping(void)
{
    t425_core_t core = {0};
    t425_core_phy_t phy = {0};
    mock_t mock = {0};
    t425_core_phy_ops_t ops = mock_ops(&mock);

    assert(t425_core_attach_phy(&core, &phy));
    assert(t425_core_phy_init(&phy, &ops, 4u));
    core.t425_channel_select = 0x8000001Cu;
    core.t425_channel_data_reg = 0xA5u;

    for (uint32_t link = 0u; link < 4u; ++link) {
        mock.tx_calls = 0u;
        assert(t425_core_link_output(&core, link) == 0);
        assert(mock.tx_calls == 1u);
        assert(mock.tx_endpoint == link);
        assert(mock.tx_frame == 0xA5u);
        assert(core.t425_channel_select == 0x8000001Cu);
    }
}

static void test_rx_peek_and_single_consumption(void)
{
    t425_core_t core = {0};
    t425_core_phy_t phy = {0};
    mock_t mock = {0};
    t425_core_phy_ops_t ops = mock_ops(&mock);
    uint8_t byte = 0u;

    assert(t425_core_attach_phy(&core, &phy));
    assert(t425_core_phy_init(&phy, &ops, 4u));
    for (uint32_t link = 0u; link < 4u; ++link) {
        mock.rx_pending[link] = true;
        mock.rx_frame[link] = 0x10u + link;
        { unsigned gets_before = mock.rx_get_calls;
        assert(t425_core_link_input_available(&core, link));
        assert(mock.rx_get_calls == gets_before);
        }
        assert(t425_core_link_input_consume(&core, link, &byte) == 0);
        assert(byte == (uint8_t)(0x10u + link));
        assert(core.t425_channel_data_reg == (uint32_t)byte);
        assert(!mock.rx_pending[link]);
        assert(!t425_core_link_input_available(&core, link));
    }
    assert(mock.rx_available_calls == 8u);
    assert(mock.rx_get_calls == 4u);
}

static void test_no_phy_invalid_and_width(void)
{
    t425_core_t core = {0};
    t425_core_phy_t phy = {0};
    mock_t mock = {0};
    t425_core_phy_ops_t ops = mock_ops(&mock);
    uint8_t byte = 0u;

    core.t425_channel_data_reg = 0x100u;
    assert(t425_core_attach_phy(&core, 0));
    assert(t425_core_link_output(&core, 0u) == -1);
    assert(!t425_core_link_input_available(&core, 0u));
    assert(t425_core_link_input_consume(&core, 0u, &byte) == -1);
    assert(t425_core_link_output(&core, 4u) == -1);
    assert(!t425_core_link_input_available(&core, 4u));
    assert(t425_core_link_input_consume(&core, 4u, &byte) == -1);

    assert(t425_core_attach_phy(&core, &phy));
    assert(t425_core_phy_init(&phy, &ops, 4u));
    assert(t425_core_link_output(&core, 0u) == -1);
}

int main(void)
{
    test_tx_mapping();
    test_rx_peek_and_single_consumption();
    test_no_phy_invalid_and_width();
    puts("F_PHY-70B CORE link seam: PASS");
    return 0;
}
