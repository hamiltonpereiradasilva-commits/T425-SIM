#include <assert.h>
#include <stdio.h>
#include "t425_reliable_v2.h"

int main(void){
 t425_rl2_t r; t425_packet_t p={.destination=3,.source=0,.ttl=8,.length=5,.payload={'H','E','L','L','O'}};
 uint8_t seq; t425_rl2_init(&r,2,10);

 /* First transaction: deadline semantics. */
 assert(t425_rl2_begin(&r,&p,100,&seq));
 assert(r.state==T425_RL2_WAIT_ACK && r.tx_time==100 && r.deadline==110);
 t425_rl2_poll(&r,109); assert(r.state==T425_RL2_WAIT_ACK);
 t425_rl2_poll(&r,110); assert(r.state==T425_RL2_RETRY);

 /* Retry must preserve packet and sequence, and establish a new deadline. */
 t425_packet_t retry; uint8_t rs;
 assert(t425_rl2_take_retry(&r,&retry,&rs));
 assert(r.state==T425_RL2_WAIT_ACK && rs==seq);
 assert(retry.sequence==seq && retry.destination==3 && retry.length==5);
 assert(r.deadline==120);

 /* Second timeout creates the second retry. */
 t425_rl2_poll(&r,119); assert(r.state==T425_RL2_WAIT_ACK);
 t425_rl2_poll(&r,120); assert(r.state==T425_RL2_RETRY);
 assert(t425_rl2_take_retry(&r,&retry,&rs));
 assert(r.deadline==130);

 /* Matching ACK completes transaction. */
 assert(t425_rl2_ack(&r,seq,121));
 assert(r.state==T425_RL2_IDLE && !r.pending_valid && r.ack_ok==1);

 /* Wrong ACK cannot close a transaction. */
 assert(t425_rl2_begin(&r,&p,200,&seq));
 assert(!t425_rl2_ack(&r,(uint8_t)(seq+1),205));
 assert(r.state==T425_RL2_WAIT_ACK && r.pending_valid);

 /* Exhaust retries: initial + two retries, then FAILED. */
 t425_rl2_poll(&r,210); assert(r.state==T425_RL2_RETRY);
 assert(t425_rl2_take_retry(&r,&retry,&rs));
 t425_rl2_poll(&r,220); assert(r.state==T425_RL2_RETRY);
 assert(t425_rl2_take_retry(&r,&retry,&rs));
 t425_rl2_poll(&r,230); assert(r.state==T425_RL2_FAILED && r.failures==1);

 puts("T425 RELIABLE DEADLINE/RETRY: PASS");
 return 0;
}
