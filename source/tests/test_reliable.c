#include <assert.h>
#include <stdio.h>
#include "t425_reliable.h"
int main(void){
 t425_reliable_t r; uint8_t s;
 t425_rl_init(&r,2,10);
 assert(t425_rl_begin(&r,100,&s)&&s==0&&r.state==T425_RL_WAIT_ACK);
 assert(!t425_rl_ack(&r,7,105));
 t425_rl_poll(&r,109);assert(r.state==T425_RL_WAIT_ACK);
 t425_rl_poll(&r,110);assert(r.state==T425_RL_RETRY&&r.retries==1);
 assert(t425_rl_take_retry(&r,&s)&&s==0);
 t425_rl_poll(&r,121);assert(r.state==T425_RL_RETRY);
 assert(t425_rl_take_retry(&r,&s));
 t425_rl_poll(&r,132);assert(r.state==T425_RL_FAILED&&r.failures==1);
 assert(t425_rl_begin(&r,200,&s)==false); /* failed until reinit */
 t425_rl_init(&r,2,10);
 assert(t425_rl_begin(&r,200,&s)&&s==0);
 assert(t425_rl_ack(&r,0,205));
 assert(r.state==T425_RL_IDLE&&r.rx_ok==1);
 puts("T425 RELIABLE LINK: PASS");return 0;
}
