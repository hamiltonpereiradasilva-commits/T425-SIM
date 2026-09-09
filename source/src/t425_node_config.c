#include "t425_node_config.h"

void t425_node_config_init(t425_node_config_t *cfg, uint8_t node_id) {
    if (!cfg) return;
    cfg->node_id = node_id;
    t425_interfaces_init(&cfg->interfaces);
}

bool t425_node_config_validate(const t425_node_config_t *cfg) {
    if (!cfg) return false;
    if (cfg->interfaces.count > T425_MAX_INTERFACES) return false;

    for (uint8_t i = 0; i < cfg->interfaces.count; ++i) {
        const t425_interface_t *it = &cfg->interfaces.items[i];

        if (it->if_id != i) return false;
        if (it->peer_id == 0xFFu) return false;
        if (it->pio_sm > 7u) return false;
        if (it->tx_gpio > 29u || it->rx_gpio > 29u) return false;
        if (it->state != T425_IF_DOWN && it->state != T425_IF_UP)
            return false;

        for (uint8_t j = 0; j < i; ++j) {
            if (cfg->interfaces.items[j].peer_id == it->peer_id)
                return false;
        }
    }
    return true;
}

bool t425_node_config_add_interface(t425_node_config_t *cfg,
                                    uint8_t peer_id, uint8_t pio_sm,
                                    uint8_t tx_gpio, uint8_t rx_gpio) {
    if (!cfg) return false;
    return t425_interface_add(&cfg->interfaces, peer_id, pio_sm,
                              tx_gpio, rx_gpio);
}
