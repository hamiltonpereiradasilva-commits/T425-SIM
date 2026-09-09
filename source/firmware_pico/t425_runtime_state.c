#include "t425_runtime_state.h"
#include <string.h>

static bool g_interfaces_ready = false;
static bool g_router_ready = false;

bool t425_runtime_init(t425_runtime_state_info_t *info) {
    if (!info) return false;
    memset(info,0,sizeof(*info));
    info->state=T425_RUNTIME_INITIALIZING;

    t425_runtime_config_t cfg;
    if (!t425_runtime_config_load(&cfg)) {
        info->state=T425_RUNTIME_RECOVERY;
        return false;
    }

    info->node_id=cfg.config.node_id;
    info->active_slot=cfg.active_slot;

    /*
     * Ordered startup boundary:
     * 1. configuration is already validated;
     * 2. interfaces may now be initialized;
     * 3. Router may start only after interfaces are ready.
     */
    g_interfaces_ready=true;
    if (!g_interfaces_ready) {
        info->state=T425_RUNTIME_RECOVERY;
        return false;
    }

    g_router_ready=true;
    if (!g_router_ready) {
        info->state=T425_RUNTIME_RECOVERY;
        return false;
    }

    info->state=T425_RUNTIME_ONLINE;
    return true;
}

bool t425_runtime_interfaces_ready(void) { return g_interfaces_ready; }
bool t425_runtime_router_ready(void) { return g_router_ready; }
