#include "t425_pio_link.h"
#include <stddef.h>
#include "hardware/clocks.h"
#include "hardware/pio.h"

bool t425_pio_link_init(t425_pio_link_t*l,PIO pio,uint tx,uint rx,
                        uint ptx,uint prx,uint baud){
    if(!l||!pio||baud==0||tx>=4||rx>=4)return false;
    l->pio=pio;l->sm_tx=tx;l->sm_rx=rx;l->pin_tx=ptx;l->pin_rx=prx;
    l->baud=baud;l->tx_program_offset=-1;l->rx_program_offset=-1;l->initialized=true;
    return true;
}
bool t425_pio_link_put(t425_pio_link_t*l,uint8_t byte){
    if(!l||!l->initialized)return false;
    pio_sm_put_blocking(l->pio,l->sm_tx,(uint32_t)byte);
    return true;
}
bool t425_pio_link_get(t425_pio_link_t*l,uint8_t*byte){
    if(!l||!l->initialized||!byte)return false;
    *byte=(uint8_t)pio_sm_get_blocking(l->pio,l->sm_rx);
    return true;
}
