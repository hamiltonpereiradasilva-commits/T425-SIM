#include "t425_reliable.h"
#include <string.h>
static uint8_t seq_inc(uint8_t x){ return (uint8_t)(x+1u); }
void t425_rl_init(t425_reliable_t*r,uint8_t max,uint32_t timeout){
    memset(r,0,sizeof(*r));
    r->state=T425_RL_IDLE;
    r->max_retries=max>T425_RL_MAX_RETRIES?T425_RL_MAX_RETRIES:max;
    r->ack_timeout=timeout;
}
bool t425_rl_begin(t425_reliable_t*r,uint32_t now,uint8_t*seq){
    if(!r||!seq||r->state!=T425_RL_IDLE)return false;
    r->pending_seq=r->next_seq;r->next_seq=seq_inc(r->next_seq);
    r->retries=0;r->deadline=now+r->ack_timeout;r->state=T425_RL_WAIT_ACK;
    *seq=r->pending_seq;return true;
}
bool t425_rl_ack(t425_reliable_t*r,uint8_t seq,uint32_t now){
    if(!r)return false;
    if(r->state==T425_RL_WAIT_ACK && seq==r->pending_seq){
        r->state=T425_RL_IDLE;r->deadline=now;r->rx_ok++;return true;
    }
    r->duplicates++;return false;
}
void t425_rl_poll(t425_reliable_t*r,uint32_t now){
    if(!r||r->state!=T425_RL_WAIT_ACK)return;
    if((int32_t)(now-r->deadline)<0)return;
    if(r->retries<r->max_retries){
        r->retries++;r->retries_total++;r->state=T425_RL_RETRY;
    }else{
        r->state=T425_RL_FAILED;r->failures++;
    }
}
bool t425_rl_should_retry(const t425_reliable_t*r){return r&&r->state==T425_RL_RETRY;}
bool t425_rl_take_retry(t425_reliable_t*r,uint8_t*seq){
    if(!r||!seq||r->state!=T425_RL_RETRY)return false;
    *seq=r->pending_seq;r->deadline=0;r->state=T425_RL_WAIT_ACK;return true;
}
void t425_rl_tx_result(t425_reliable_t*r,bool ok){
    if(!r)return;
    if(ok)r->tx_ok++;
}
