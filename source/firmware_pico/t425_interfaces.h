#pragma once
#include <stdint.h>
#include <stdbool.h>

#define T425_MAX_INTERFACES 4
/* Compatibility name for runtime/link layers. */
#define T425_MAX_LINKS T425_MAX_INTERFACES

typedef enum {
    T425_IF_DOWN = 0,
    T425_IF_UP = 1
} t425_if_state_t;

typedef struct {
    uint8_t if_id;
    uint8_t peer_id;
    uint8_t pio_sm;
    uint8_t tx_gpio;
    uint8_t rx_gpio;
    t425_if_state_t state;
    uint32_t rx_frames;
    uint32_t tx_frames;
} t425_interface_t;

typedef struct {
    t425_interface_t items[T425_MAX_INTERFACES];
    uint8_t count;
} t425_interface_table_t;

void t425_interfaces_init(t425_interface_table_t *t);
bool t425_interface_add(t425_interface_table_t *t, uint8_t peer_id,
                         uint8_t pio_sm, uint8_t tx_gpio, uint8_t rx_gpio);
bool t425_interface_set_state(t425_interface_table_t *t, uint8_t if_id,
                              t425_if_state_t state);
int t425_interface_for_peer(const t425_interface_table_t *t, uint8_t peer_id);
