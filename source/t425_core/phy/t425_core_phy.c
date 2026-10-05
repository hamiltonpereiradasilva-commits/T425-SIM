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

bool t425_core_phy_read_system_inputs(
    t425_core_phy_t *phy,
    t425_core_phy_system_inputs_t *inputs)
{
    if (!t425_core_phy_endpoint_valid(phy, 0u) ||
        inputs == 0 ||
        phy->ops.read_system_inputs == 0)
        return false;

    return phy->ops.read_system_inputs(phy->ops.context, inputs);
}

bool t425_core_phy_write_system_outputs(
    t425_core_phy_t *phy,
    const t425_core_phy_system_outputs_t *outputs)
{
    if (!t425_core_phy_endpoint_valid(phy, 0u) ||
        outputs == 0 ||
        phy->ops.write_system_outputs == 0)
        return false;

    return phy->ops.write_system_outputs(phy->ops.context, outputs);
}

int t425_core_phy_set_rx_admission(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    bool enabled)
{
    if (!t425_core_phy_endpoint_valid(phy, endpoint_id) ||
        phy->ops.set_rx_admission == 0)
        return -1;

    return phy->ops.set_rx_admission(
        phy->ops.context, endpoint_id, enabled);
}

int t425_core_phy_set_rx_admission_credit(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint32_t generation,
    bool enabled)
{
    if (!t425_core_phy_endpoint_valid(phy, endpoint_id) ||
        phy->ops.set_rx_admission_credit == 0)
        return -1;

    return phy->ops.set_rx_admission_credit(
        phy->ops.context, endpoint_id, generation, enabled);
}
