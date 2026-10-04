#include "t425_core_phy_bootstrap.h"

static bool t425_core_phy_bootstrap_ops_valid(
    const t425_core_phy_ops_t *ops)
{
    return ops != 0 &&
           ops->tx_frame != 0 &&
           ops->rx_available != 0 &&
           ops->rx_get != 0;
}

bool t425_core_phy_bootstrap_init(
    t425_core_phy_bootstrap_t *bootstrap,
    const t425_core_phy_ops_t *core1_ops)
{
    t425_core_phy_ops_t core0_ops;

    if (bootstrap == 0 ||
        bootstrap->initialized ||
        !t425_core_phy_bootstrap_ops_valid(core1_ops))
        return false;

    if (!t425_core_phy_ipc_init(&bootstrap->ipc))
        return false;

    t425_core_phy_ipc_make_core0_ops(
        &bootstrap->ipc,
        &core0_ops);

    if (!t425_core_phy_init(
            &bootstrap->core0_phy,
            &core0_ops,
            T425_CORE_PHY_MAX_ENDPOINTS))
        return false;

    if (!t425_core_phy_ipc_service_init(
            &bootstrap->core1_service,
            &bootstrap->ipc,
            core1_ops))
        return false;

    bootstrap->initialized = true;
    return true;
}

t425_core_phy_t *t425_core_phy_bootstrap_core0(
    t425_core_phy_bootstrap_t *bootstrap)
{
    if (bootstrap == 0 || !bootstrap->initialized)
        return 0;

    return &bootstrap->core0_phy;
}

uint32_t t425_core_phy_bootstrap_poll_core1(
    t425_core_phy_bootstrap_t *bootstrap)
{
    if (bootstrap == 0 || !bootstrap->initialized)
        return 0u;

    return t425_core_phy_ipc_service_poll(
        &bootstrap->core1_service);
}
