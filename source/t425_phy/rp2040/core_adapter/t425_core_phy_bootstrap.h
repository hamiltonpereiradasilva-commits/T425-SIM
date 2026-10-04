#ifndef T425_CORE_PHY_BOOTSTRAP_H
#define T425_CORE_PHY_BOOTSTRAP_H

#include <stdbool.h>
#include <stdint.h>

#include "t425_core_phy_ipc.h"

typedef struct
{
    t425_core_phy_ipc_t ipc;
    t425_core_phy_t core0_phy;
    t425_core_phy_ipc_service_t core1_service;
    bool initialized;
} t425_core_phy_bootstrap_t;

bool t425_core_phy_bootstrap_init(
    t425_core_phy_bootstrap_t *bootstrap,
    const t425_core_phy_ops_t *core1_ops);

t425_core_phy_t *t425_core_phy_bootstrap_core0(
    t425_core_phy_bootstrap_t *bootstrap);

uint32_t t425_core_phy_bootstrap_poll_core1(
    t425_core_phy_bootstrap_t *bootstrap);

#endif
