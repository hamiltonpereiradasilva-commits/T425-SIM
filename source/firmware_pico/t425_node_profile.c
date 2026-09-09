#include "t425_node_profile.h"

bool t425_node_profile_apply(const t425_node_profile_t *p,
                             t425_node_config_t *cfg) {
    if (!p || !cfg || p->interface_count > T425_MAX_INTERFACES)
        return false;

    t425_node_config_init(cfg, p->node_id);
    for (uint8_t i = 0; i < p->interface_count; ++i) {
        if (!t425_node_config_add_interface(
                cfg, p->interfaces[i].peer_id,
                p->interfaces[i].pio_sm,
                p->interfaces[i].tx_gpio,
                p->interfaces[i].rx_gpio))
            return false;
    }
    return true;
}
