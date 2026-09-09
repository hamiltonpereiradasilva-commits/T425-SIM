#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_node_config.h"
#include "t425_pio_link.h"

typedef struct {
    t425_node_config_t config;
    t425_pio_link_t pio_links[T425_MAX_LINKS];
    uint8_t active_links;
    uint32_t tx_bytes;
    uint32_t rx_bytes;
    uint32_t tx_frames;
    uint32_t rx_frames;
    uint32_t rx_errors;
} t425_link_manager_t;

bool t425_link_manager_init(t425_link_manager_t *m,
                            const t425_node_config_t *config);
bool t425_link_manager_send_byte(t425_link_manager_t *m,
                                 uint8_t local_link, uint8_t byte);
bool t425_link_manager_recv_byte(t425_link_manager_t *m,
                                 uint8_t local_link, uint8_t *byte);
bool t425_link_manager_link_enabled(const t425_link_manager_t *m,
                                    uint8_t local_link);
