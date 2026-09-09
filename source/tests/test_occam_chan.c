#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_occam_chan.h"
int main(void){
 t425_occam_chan_t a,b;t425_occam_message_t m;t425_link_packet_t p;
 uint8_t x[]={1,2,3,4};
 t425_occam_chan_init(&a,0,1,7);t425_occam_chan_init(&b,0,1,7);
 assert(t425_occam_encode_out(&a,x,4,&p));
 assert(p.type==T425_LOUT&&p.payload[0]==7);
 assert(t425_occam_decode_in(&b,&p,&m));
 assert(m.chan_id==7&&m.len==4&&!memcmp(m.value,x,4));
 puts("OCCAM CHAN OUT/IN: PASS");return 0;
}
