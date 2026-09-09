#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "t425_interfaces.h"

#define T425_MAX_NODE_ID 255

typedef struct {
    uint8_t node_id;
    t425_interface_table_t interfaces;
} t425_node_config_t;

bool t425_node_config_validate(const t425_node_config_t *cfg);

void t425_node_config_init(
    t425_node_config_t *cfg,
    uint8_t node_id
);

bool t425_node_config_add_interface(
    t425_node_config_t *cfg,
    uint8_t peer_id,
    uint8_t pio_sm,
    uint8_t tx_gpio,
    uint8_t rx_gpio
);
