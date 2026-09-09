#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_node_config.h"

/*
 * Generic firmware profile.
 * The protocol/application is node-independent.
 * Only the startup configuration identifies a node and its physical peers.
 */
typedef struct {
    uint8_t node_id;
    uint8_t interface_count;
    struct {
        uint8_t peer_id;
        uint8_t pio_sm;
        uint8_t tx_gpio;
        uint8_t rx_gpio;
    } interfaces[T425_MAX_INTERFACES];
} t425_node_profile_t;

bool t425_node_profile_apply(const t425_node_profile_t *p,
                             t425_node_config_t *cfg);
