#include <assert.h>
#include <stdio.h>
#include "t425_ack.h"
int main(void){
 t425_ack_ctrl_t a;
 t425_ack_init(&a,3,10);
 assert(t425_ack_start(&a,17,100));
 assert(!t425_ack_poll(&a,109));
 assert(t425_ack_receive(&a,17));
 assert(a.acked==1&&a.state==T425_TX_IDLE);

 assert(t425_ack_start(&a,18,200));
 assert(t425_ack_poll(&a,210)&&t425_ack_can_retry(&a));
 a.state=T425_TX_WAIT_ACK;a.deadline=220;
 assert(t425_ack_poll(&a,220)&&t425_ack_can_retry(&a));
 a.state=T425_TX_WAIT_ACK;a.deadline=230;
 assert(t425_ack_poll(&a,230)&&t425_ack_can_retry(&a));
 a.state=T425_TX_WAIT_ACK;a.deadline=240;
 assert(t425_ack_poll(&a,240)&&a.state==T425_TX_FAILED);
 assert(a.timeouts==4&&a.retries_sent==3&&a.failed==1);
 puts("ACK TIMEOUT RETRY FSM: PASS"); return 0;
}
