#include "t425_remote_rdv.h"
#include <string.h>
static void pkt(t425_remote_rdv_t*r,uint8_t dst,uint8_t type,uint8_t seq,
                uint8_t chan,const uint8_t*d,uint8_t n,t425_link_packet_t*p){
 memset(p,0,sizeof(*p));p->sof=T425_LINK_SOF;p->src=r->node_id;p->dst=dst;
 p->type=type;p->seq=seq;p->len=(uint8_t)(n+1);p->payload[0]=chan;
 if(n)memcpy(&p->payload[1],d,n);
}
void t425_remote_rdv_init(t425_remote_rdv_t*r,uint8_t id){memset(r,0,sizeof(*r));r->node_id=id;}
bool t425_remote_rdv_make_out(t425_remote_rdv_t*r,uint8_t dst,uint8_t chan,
 const uint8_t*d,uint8_t n,t425_link_packet_t*p){
 if(!r||!p||r->pending||n>T425_OCCAM_MAX_VALUE||(!d&&n))return false;
 uint8_t s=r->next_seq++;pkt(r,dst,T425_RDV_REQ,s,chan,d,n,p);
 r->pending=true;r->pending_seq=s;r->pending_chan=chan;r->pending_dst=dst;return true;
}
bool t425_remote_rdv_make_ack(t425_remote_rdv_t*r,uint8_t dst,uint8_t chan,
 uint8_t seq,t425_link_packet_t*p){
 if(!r||!p) return false;
 pkt(r,dst,T425_RDV_ACK,seq,chan,0,0,p);
 return true;
}
bool t425_remote_rdv_handle(t425_remote_rdv_t*r,const t425_link_packet_t*in,t425_link_packet_t*reply){
 if(!r||!in)return false;
 if(in->type==T425_RDV_ACK&&r->pending&&in->seq==r->pending_seq&&
    in->src==r->pending_dst&&in->len>=1&&in->payload[0]==r->pending_chan){
   r->pending=false;return true;
 }
 if(in->type==T425_RDV_REQ&&reply&&in->len>=1){
   return t425_remote_rdv_make_ack(r,in->src,in->payload[0],in->seq,reply);
 }
 return false;
}
