#include "t425_reliable_v2.h"
#include <string.h>

static uint8_t inc8(uint8_t x){ return (uint8_t)(x+1u); }

void t425_rl2_init(t425_rl2_t*r,uint8_t max,uint32_t timeout){
 memset(r,0,sizeof(*r));
 r->state=T425_RL2_IDLE;
 r->max_retries=max>T425_RL2_MAX_RETRIES?T425_RL2_MAX_RETRIES:max;
 r->ack_timeout=timeout?timeout:T425_RL2_TIMEOUT;
}

bool t425_rl2_begin(t425_rl2_t*r,t425_packet_t*p,uint32_t now,uint8_t*seq){
 if(!r||!p||!seq||r->state!=T425_RL2_IDLE)return false;
 r->pending_packet=*p;
 r->pending_seq=r->next_seq;
 r->next_seq=inc8(r->next_seq);
 r->pending_packet.sequence=r->pending_seq;
 r->retries=0;
 r->tx_time=now;
 r->deadline=now+r->ack_timeout;
 r->pending_valid=true;
 r->state=T425_RL2_WAIT_ACK;
 *seq=r->pending_seq;
 return true;
}

bool t425_rl2_ack(t425_rl2_t*r,uint8_t seq,uint32_t now){
 if(!r)return false;
 if(r->state==T425_RL2_WAIT_ACK && r->pending_valid && seq==r->pending_seq){
  r->state=T425_RL2_IDLE;
  r->pending_valid=false;
  r->ack_ok++;
  r->tx_time=now;
  r->deadline=now;
  return true;
 }
 r->duplicates++;
 return false;
}

void t425_rl2_poll(t425_rl2_t*r,uint32_t now){
 if(!r || r->state!=T425_RL2_WAIT_ACK || !r->pending_valid)return;
 /* Unsigned wrap-safe deadline comparison: true when now reached/passed deadline. */
 if((int32_t)(now-r->deadline)<0)return;
 if(r->retries<r->max_retries){
  r->retries++;
  r->retries_total++;
  r->state=T425_RL2_RETRY;
 } else {
  r->state=T425_RL2_FAILED;
  r->failures++;
 }
}

bool t425_rl2_take_retry(t425_rl2_t*r,t425_packet_t*p,uint8_t*seq){
 if(!r||!p||!seq||r->state!=T425_RL2_RETRY||!r->pending_valid)return false;
 *p=r->pending_packet;
 *seq=r->pending_seq;
 r->tx_time=r->deadline;
 r->deadline=r->tx_time+r->ack_timeout;
 r->state=T425_RL2_WAIT_ACK;
 return true;
}

void t425_rl2_tx_result(t425_rl2_t*r,bool ok){
 if(r&&ok)r->tx_ok++;
}
