#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_link_loopback.h"

typedef struct { t425_link_endpoint_t *peer; int drop_ack; unsigned tx; } bus_t;
static bool wire(uint8_t link,const uint8_t*d,uint16_t n,void*c){
 bus_t*b=(bus_t*)c;(void)link;b->tx++;
 if(b->drop_ack){
   t425_packet_t q;
   if(t425_transport_decode(d,n,&q)&&q.destination==T425_ACK_DEST)return true;
 }
 return t425_link_receive(b->peer,d,n,100);
}
int main(void){
 t425_link_endpoint_t a,b;bus_t ba={0},bb={0};
 t425_link_endpoint_init(&a,1,0,2,10,wire,&ba);
 t425_link_endpoint_init(&b,1,1,2,10,wire,&bb);
 ba.peer=&b;bb.peer=&a;
 t425_packet_t p={.destination=1,.source=0,.ttl=8,.length=5,.payload={'H','E','L','L','O'}};
 assert(t425_link_send(&a,&p,100));
 assert(a.rl.state==T425_RL2_IDLE && a.rl.ack_ok==1);
 assert(b.rx_data==1 && b.rx_ack==0);

 /* Force ACK loss, then verify retry reuses the pending sequence. */
 ba.drop_ack=1;
 p.sequence=99;
 assert(t425_link_send(&a,&p,200));
 assert(a.rl.state==T425_RL2_WAIT_ACK);
 t425_link_poll(&a,210);
 assert(a.rl.state==T425_RL2_RETRY);
 t425_link_poll(&a,210);
 assert(a.rl.state==T425_RL2_WAIT_ACK);
 assert(a.rl.pending_packet.sequence==1);
 ba.drop_ack=0;
 assert(t425_link_receive(&a,(const uint8_t[]){0},1,211)==false);
 puts("T425 ONE-HOP LINK INTEGRATION: PASS");
 return 0;
}
