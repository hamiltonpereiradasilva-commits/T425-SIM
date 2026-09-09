#include <assert.h>
#include <stdio.h>
#include "t425_reliable_v2.h"
int main(void){
 t425_rl2_t r;t425_packet_t p={.destination=3,.source=0,.ttl=8,.length=5,.payload={'H','E','L','L','O'}};
 uint8_t seq; t425_rl2_init(&r,2,10);
 assert(t425_rl2_begin(&r,&p,100,&seq));assert(seq==0&&r.pending_valid);
 assert(r.pending_packet.destination==3&&r.pending_packet.sequence==0);
 t425_rl2_poll(&r,110);assert(r.state==T425_RL2_RETRY);
 t425_packet_t retry;uint8_t s2;assert(t425_rl2_take_retry(&r,&retry,&s2));
 assert(s2==0&&retry.destination==3&&retry.length==5);
 assert(!t425_rl2_ack(&r,9,115));
 assert(t425_rl2_ack(&r,0,116));assert(r.state==T425_RL2_IDLE&&!r.pending_valid);
 assert(t425_rl2_begin(&r,&p,200,&seq));t425_rl2_poll(&r,210);assert(t425_rl2_take_retry(&r,&retry,&s2));
 t425_rl2_poll(&r,220);assert(r.state==T425_RL2_RETRY);assert(t425_rl2_take_retry(&r,&retry,&s2));
 t425_rl2_poll(&r,230);assert(r.state==T425_RL2_FAILED&&r.failures==1);
 puts("T425 RELIABLE RETRANSMISSION CACHE: PASS");return 0;
}
