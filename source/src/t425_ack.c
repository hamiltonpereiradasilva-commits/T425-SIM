#include "t425_ack.h"
#include <string.h>

void t425_ack_init(t425_ack_ctrl_t*a,uint8_t max,uint32_t timeout){
    memset(a,0,sizeof(*a)); a->state=T425_TX_IDLE;
    a->max_retries=max; a->timeout_ticks=timeout;
}
bool t425_ack_start(t425_ack_ctrl_t*a,uint8_t seq,uint32_t now){
    if(!a||a->state!=T425_TX_IDLE)return false;
    a->seq=seq;a->retries=0;a->deadline=now+a->timeout_ticks;
    a->state=T425_TX_WAIT_ACK;return true;
}
bool t425_ack_receive(t425_ack_ctrl_t*a,uint8_t seq){
    if(!a||a->state!=T425_TX_WAIT_ACK||seq!=a->seq)return false;
    a->acked++;a->state=T425_TX_IDLE;return true;
}
bool t425_ack_poll(t425_ack_ctrl_t*a,uint32_t now){
    if(!a||a->state!=T425_TX_WAIT_ACK)return false;
    if((int32_t)(now-a->deadline)<0)return false;
    a->timeouts++;
    if(a->retries<a->max_retries){
        a->retries++;a->retries_sent++;a->deadline=now+a->timeout_ticks;
        a->state=T425_TX_RETRY;return true;
    }
    a->failed++;a->state=T425_TX_FAILED;return true;
}
bool t425_ack_can_retry(const t425_ack_ctrl_t*a){
    return a&&a->state==T425_TX_RETRY;
}
