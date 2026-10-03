#include "t425_core_phy_adapter.h"

int t425_core_phy_adapter_init(
    t425_core_phy_adapter_t *adapter,
    t425_link_t *const links[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    const t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT])
{
    return t425_core_phy_rp2040_init(adapter, links, routes) ? 0 : -1;
}

void t425_core_phy_adapter_make_ops(
    t425_core_phy_adapter_t *adapter,
    t425_core_phy_ops_t *ops)
{
    t425_core_phy_rp2040_make_ops(adapter, ops);
}
