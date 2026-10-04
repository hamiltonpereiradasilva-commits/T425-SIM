#include "t425_core_phy.h"

static bool t425_core_phy_endpoint_valid(
    const t425_core_phy_t *phy,
    uint32_t endpoint_id)
{
    return phy != 0 &&
           phy->attached &&
           endpoint_id < phy->endpoint_count;
}

bool t425_core_phy_init(
    t425_core_phy_t *phy,
    const t425_core_phy_ops_t *ops,
    uint32_t endpoint_count)
{
    if (phy == 0 || ops == 0)
        return false;

    if (endpoint_count == 0u ||
        endpoint_count > T425_CORE_PHY_MAX_ENDPOINTS)
        return false;

    if (ops->tx_frame == 0 ||
        ops->rx_available == 0 ||
        ops->rx_get == 0)
        return false;

    phy->ops = *ops;
    phy->endpoint_count = endpoint_count;
    phy->attached = true;
    return true;
}

int t425_core_phy_tx(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint16_t frame)
{
    if (!t425_core_phy_endpoint_valid(phy, endpoint_id))
        return -1;

    if (frame > 0xFFu)
        return -1;

    return phy->ops.tx_frame(
        phy->ops.context,
        endpoint_id,
        frame);
}

bool t425_core_phy_rx_available(
    t425_core_phy_t *phy,
    uint32_t endpoint_id)
{
    if (!t425_core_phy_endpoint_valid(phy, endpoint_id))
        return false;

    return phy->ops.rx_available(
        phy->ops.context,
        endpoint_id);
}

int t425_core_phy_rx(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint32_t *frame)
{
    if (!t425_core_phy_endpoint_valid(phy, endpoint_id) ||
        frame == 0)
        return -1;

    return phy->ops.rx_get(
        phy->ops.context,
        endpoint_id,
        frame);
}
