#include <assert.h>
#include <stdio.h>
#include "t425_pio_transport.h"
int main(void){t425_pio_link_cfg_t c={2,3,0,0,1,false};uint8_t d[3]={1,2,3};
assert(t425_pio_transport_init(0,&c));assert(t425_pio_transport_write(0,d,3));assert(!t425_pio_transport_init(2,&c));
puts("PIO transport API: PASS");return 0;}
