#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_link_loopback.h"
typedef struct {t425_link_endpoint_t *peer; unsigned tx; unsigned delivered;} bus_t;
static bool wire(uint8_t link,const uint8_t*d,uint16_t n,void*c){
    bus_t*b=(bus_t*)c;(void)link;b->tx++;
    return t425_link_receive(b->peer,d,n,100);
}
int main(void){
 t425_link_endpoint_t a,b;bus_t ba={0},bb={0};
 t425_link_endpoint_init(&a,1,2,10,wire,&ba);
 t425_link_endpoint_init(&b,1,2,10,wire,&bb);
 ba.peer=&b;bb.peer=&a;
 t425_packet_t p={.destination=1,.source=0,.ttl=8,.sequence=0,.length=5,.payload={'H','E','L','L','O'}};
 assert(t425_link_send(&a,&p,100));assert(a.rl.state==T425_RL2_IDLE);
 assert(b.rx_frames==1&&a.rl.ack_ok==1&&ba.tx>=2);
 uint8_t bad[16]={0};assert(!t425_link_receive(&a,bad,sizeof(bad),101));assert(a.rx_bad_crc==1);
 puts("T425 ONE-HOP PHY/ACK LOOPBACK: PASS");return 0;
}
