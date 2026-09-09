#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_reliable_link.h"
int main(void){
 t425_reliable_link_t a,b;t425_reliable_init(&a,0);t425_reliable_init(&b,1);
 t425_link_packet_t d,ack;uint8_t x[]={7,8,9};
 assert(t425_reliable_make_data(&a,1,x,3,&d));
 assert(a.waiting_ack&&d.seq==0);
 assert(t425_reliable_on_packet(&b,&d,&ack));
 assert(ack.type==T425_LACK&&ack.seq==d.seq);
 assert(t425_reliable_on_packet(&a,&ack,0));
 assert(!a.waiting_ack);
 assert(t425_reliable_make_data(&a,1,x,3,&d));
 assert(t425_reliable_timeout(&a));assert(a.retransmissions==1);
 assert(t425_reliable_timeout(&a));assert(t425_reliable_timeout(&a));
 assert(!t425_reliable_timeout(&a));assert(a.timeouts==1&&!a.waiting_ack);
 puts("ACK/timeout/retry: PASS");return 0;
}
