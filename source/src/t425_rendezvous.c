#include "t425_rendezvous.h"
#include <string.h>
void t425_rdv_init(t425_rendezvous_t*r){
 memset(r,0,sizeof(*r));r->state=T425_RDV_EMPTY;r->out_proc=-1;r->in_proc=-1;
}
bool t425_rdv_post_out(t425_rendezvous_t*r,uint8_t proc,const uint8_t*d,uint8_t n){
 if(!r||n>T425_OCCAM_MAX_VALUE||(!d&&n))return false;
 if(r->state==T425_RDV_WAIT_IN){
   r->out_proc=(int8_t)proc;r->pending.len=n;
   if(n)memcpy(r->pending.value,d,n);
   r->state=T425_RDV_TRANSFER;return true;
 }
 if(r->state!=T425_RDV_EMPTY)return false;
 r->out_proc=(int8_t)proc;r->pending.len=n;
 if(n)memcpy(r->pending.value,d,n);
 r->state=T425_RDV_WAIT_OUT;return true;
}
bool t425_rdv_post_in(t425_rendezvous_t*r,uint8_t proc){
 if(!r)return false;
 if(r->state==T425_RDV_WAIT_OUT){r->in_proc=(int8_t)proc;r->state=T425_RDV_TRANSFER;return true;}
 if(r->state!=T425_RDV_EMPTY)return false;
 r->in_proc=(int8_t)proc;r->state=T425_RDV_WAIT_IN;return true;
}
bool t425_rdv_transfer(t425_rendezvous_t*r,t425_occam_message_t*m,uint8_t*op,uint8_t*ip){
 if(!r||!m||!op||!ip||r->state!=T425_RDV_TRANSFER)return false;
 *m=r->pending;m->chan_id=0;*op=(uint8_t)r->out_proc;*ip=(uint8_t)r->in_proc;
 r->state=T425_RDV_DONE;return true;
}
