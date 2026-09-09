#include "t425_occam_runtime.h"
#include <string.h>

void t425_occam_runtime_init(t425_occam_runtime_t*r){
    memset(r,0,sizeof(*r));
    for(int i=0;i<T425_OCCAM_MAX_CHANS;i++){r->chan[i].waiting_in=-1;r->chan[i].waiting_out=-1;}
}
int t425_occam_proc_create(t425_occam_runtime_t*r,uint8_t priority){
    if(!r||r->proc_count>=T425_OCCAM_MAX_PROCS)return -1;
    uint8_t id=r->proc_count++;
    r->proc[id]=(t425_occam_proc_t){.proc_id=id,.state=T425_PROC_READY,.pc=0,.priority=priority};
    return id;
}
bool t425_occam_chan_create(t425_occam_runtime_t*r,uint8_t*id){
    if(!r||!id||r->chan_count>=T425_OCCAM_MAX_CHANS)return false;
    *id=r->chan_count++;
    r->chan[*id].waiting_in=-1;r->chan[*id].waiting_out=-1;
    return true;
}
bool t425_occam_out(t425_occam_runtime_t*r,uint8_t pid,uint8_t cid,const uint8_t*d,uint8_t n){
    if(!r||pid>=r->proc_count||cid>=r->chan_count||n>T425_OCCAM_MAX_VALUE)return false;
    t425_occam_chan_runtime_t*c=&r->chan[cid];
    if(c->count>=T425_OCCAM_QUEUE_DEPTH){
        r->proc[pid].state=T425_PROC_BLOCKED_OUT;c->waiting_out=(int8_t)pid;return false;
    }
    t425_occam_message_t*m=&c->q[c->tail];
    m->chan_id=cid;m->len=n;if(n)memcpy(m->value,d,n);
    c->tail=(uint8_t)((c->tail+1)%T425_OCCAM_QUEUE_DEPTH);c->count++;
    r->proc[pid].state=T425_PROC_READY;
    if(c->waiting_in>=0){r->proc[(uint8_t)c->waiting_in].state=T425_PROC_READY;c->waiting_in=-1;}
    return true;
}
bool t425_occam_in(t425_occam_runtime_t*r,uint8_t pid,uint8_t cid,t425_occam_message_t*m){
    if(!r||!m||pid>=r->proc_count||cid>=r->chan_count)return false;
    t425_occam_chan_runtime_t*c=&r->chan[cid];
    if(c->count==0){r->proc[pid].state=T425_PROC_BLOCKED_IN;c->waiting_in=(int8_t)pid;return false;}
    *m=c->q[c->head];c->head=(uint8_t)((c->head+1)%T425_OCCAM_QUEUE_DEPTH);c->count--;
    r->proc[pid].state=T425_PROC_READY;
    if(c->waiting_out>=0){r->proc[(uint8_t)c->waiting_out].state=T425_PROC_READY;c->waiting_out=-1;}
    return true;
}
void t425_occam_schedule(t425_occam_runtime_t*r){
    if(!r||!r->proc_count)return;
    int best=-1;
    for(uint8_t i=0;i<r->proc_count;i++)
        if(r->proc[i].state==T425_PROC_READY&&(best<0||r->proc[i].priority>r->proc[(uint8_t)best].priority))
            best=i;
    if(best>=0){for(uint8_t i=0;i<r->proc_count;i++)if(r->proc[i].state==T425_PROC_RUNNING)r->proc[i].state=T425_PROC_READY;
        r->proc[(uint8_t)best].state=T425_PROC_RUNNING;r->current=(uint8_t)best;}
}
