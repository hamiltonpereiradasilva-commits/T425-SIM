#include "t425_pio_links.h"
#include "t425_links.h"
/*
 * v1.0 hardware boundary.
 * The PIO program supplies byte-level TX/RX. The packet layer remains
 * transport-independent so the same image can be tested on the host.
 */
void t425_pio_links_init(void){ t425_links_init(); }
bool t425_pio_link_send(uint8_t l,const uint8_t*d,uint16_t n){return t425_link_send(l,d,n);}
bool t425_pio_link_receive(uint8_t l,uint8_t*d,uint16_t*n){return t425_link_receive(l,d,n);}
