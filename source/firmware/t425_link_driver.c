#include "t425_link_driver.h"
#include <stddef.h>
#include "hardware/clocks.h"

bool t425_link_init(t425_link_t *l, PIO pio, uint tx, uint rx,
                    uint ptx, uint prx, uint baud) {
    if (!l || !pio || baud == 0) return false;
    l->pio=pio; l->sm_tx=tx; l->sm_rx=rx;
    l->pin_tx=ptx; l->pin_rx=prx; l->baud=baud; l->initialized=true;
    /* PIO program installation/configuration is intentionally isolated here. */
    return true;
}
bool t425_link_write_byte(t425_link_t *l, uint8_t b) {
    if (!l || !l->initialized) return false;
    pio_sm_put_blocking(l->pio,l->sm_tx,(uint32_t)b);
    return true;
}
bool t425_link_read_byte(t425_link_t *l, uint8_t *b) {
    if (!l || !l->initialized || !b) return false;
    *b=(uint8_t)pio_sm_get_blocking(l->pio,l->sm_rx);
    return true;
}
