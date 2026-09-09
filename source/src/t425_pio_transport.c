#include "t425_pio_transport.h"
static t425_pio_link_cfg_t g_cfg[T425_PIO_LINKS];
bool t425_pio_transport_init(uint8_t link,const t425_pio_link_cfg_t*c){
    if(link>=T425_PIO_LINKS||!c)return false;
    g_cfg[link]=*c;
    g_cfg[link].initialized=true;
    return true;
}
bool t425_pio_transport_write(uint8_t link,const uint8_t*d,uint16_t n){
    (void)d;(void)n;
    return link<T425_PIO_LINKS&&g_cfg[link].initialized;
}
uint16_t t425_pio_transport_read(uint8_t link,uint8_t*d,uint16_t n){
    (void)d;(void)n;(void)link;
    return 0;
}
