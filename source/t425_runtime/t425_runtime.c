#include "t425_runtime.h"

bool t425_runtime_init(t425_runtime_t *runtime, const t425_core_phy_ops_t *core1_ops)
{
    if (runtime == 0 || runtime->initialized)
        return false;
    if (!t425_core_phy_bootstrap_init(&runtime->bootstrap, core1_ops))
        return false;
    runtime->initialized = true;
    return true;
}

t425_core_phy_t *t425_runtime_core0_phy(t425_runtime_t *runtime)
{
    if (runtime == 0 || !runtime->initialized)
        return 0;
    return t425_core_phy_bootstrap_core0(&runtime->bootstrap);
}

uint32_t t425_runtime_poll_core1(t425_runtime_t *runtime)
{
    if (runtime == 0 || !runtime->initialized)
        return 0u;
    return t425_core_phy_bootstrap_poll_core1(&runtime->bootstrap);
}
