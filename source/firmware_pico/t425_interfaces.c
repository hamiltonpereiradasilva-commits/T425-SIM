#include "t425_interfaces.h"

void t425_interfaces_init(t425_interface_table_t *t) {
    t->count = 0;
    for (unsigned i=0; i<T425_MAX_INTERFACES; ++i) {
        t->items[i] = (t425_interface_t){
            .if_id=(uint8_t)i, .peer_id=0xFF, .pio_sm=0xFF,
            .tx_gpio=0xFF, .rx_gpio=0xFF, .state=T425_IF_DOWN,
            .rx_frames=0, .tx_frames=0
        };
    }
}

bool t425_interface_add(t425_interface_table_t *t, uint8_t peer_id,
                        uint8_t pio_sm, uint8_t tx_gpio, uint8_t rx_gpio) {
    if (!t || t->count >= T425_MAX_INTERFACES) return false;
    t->items[t->count].peer_id = peer_id;
    t->items[t->count].pio_sm = pio_sm;
    t->items[t->count].tx_gpio = tx_gpio;
    t->items[t->count].rx_gpio = rx_gpio;
    t->items[t->count].state = T425_IF_UP;
    t->count++;
    return true;
}

bool t425_interface_set_state(t425_interface_table_t *t, uint8_t if_id,
                              t425_if_state_t state) {
    if (!t || if_id >= t->count) return false;
    t->items[if_id].state = state;
    return true;
}

int t425_interface_for_peer(const t425_interface_table_t *t, uint8_t peer_id) {
    if (!t) return -1;
    for (uint8_t i=0; i<t->count; ++i)
        if (t->items[i].peer_id == peer_id &&
            t->items[i].state == T425_IF_UP) return (int)i;
    return -1;
}
