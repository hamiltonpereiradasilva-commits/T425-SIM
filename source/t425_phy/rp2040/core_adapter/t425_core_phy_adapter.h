#ifndef T425_CORE_PHY_ADAPTER_H
#define T425_CORE_PHY_ADAPTER_H

/* Compatibility wrapper over the single explicit RP2040 endpoint router. */
#include "t425_core_phy_rp2040.h"

typedef t425_core_phy_rp2040_t t425_core_phy_adapter_t;

int t425_core_phy_adapter_init(
    t425_core_phy_adapter_t *adapter,
    t425_link_t *const links[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    const t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT]);

void t425_core_phy_adapter_make_ops(
    t425_core_phy_adapter_t *adapter,
    t425_core_phy_ops_t *ops);

#endif
