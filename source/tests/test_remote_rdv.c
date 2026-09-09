#include <assert.h>
#include <stdio.h>
#include "t425_remote_rdv.h"
int main(void){
 t425_remote_rdv_t a,b;t425_link_packet_t q,ack;uint8_t x[]={55,66};
 t425_remote_rdv_init(&a,0);t425_remote_rdv_init(&b,1);
 assert(t425_remote_rdv_make_out(&a,1,4,x,2,&q));
 assert(q.type==T425_RDV_REQ&&q.src==0&&q.dst==1&&q.payload[0]==4);
 assert(t425_remote_rdv_handle(&b,&q,&ack));
 assert(ack.type==T425_RDV_ACK&&ack.seq==q.seq&&ack.payload[0]==4);
 assert(t425_remote_rdv_handle(&a,&ack,0));assert(!a.pending);
 puts("REMOTE RDV REQ/ACK: PASS");return 0;
}
