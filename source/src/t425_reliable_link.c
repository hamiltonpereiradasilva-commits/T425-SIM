#include "t425_reliable_link.h"
#include <string.h>
static void initpkt(t425_reliable_link_t*l,uint8_t dst,uint8_t type,uint8_t seq,t425_link_packet_t*p){
    memset(p,0,sizeof(*p));
    p->sof=T425_LINK_SOF; p->src=l->node_id; p->dst=dst; p->type=type; p->seq=seq;
}
void t425_reliable_init(t425_reliable_link_t*l,uint8_t id){
    memset(l,0,sizeof(*l)); l->node_id=id;
}
bool t425_reliable_make_data(t425_reliable_link_t*l,uint8_t dst,const uint8_t*d,uint8_t n,t425_link_packet_t*p){
    if(!l||!p||n>T425_LINK_MAX_PAYLOAD||(!d&&n)||l->waiting_ack) return false;
    uint8_t s=l->next_seq++;
    initpkt(l,dst,T425_LDATA,s,p); p->len=n;
    if(n) memcpy(p->payload,d,n);
    l->pending_seq=s; l->pending_dst=dst; l->retries=0; l->waiting_ack=true; l->tx_packets++;
    return true;
}
bool t425_reliable_make_ack(t425_reliable_link_t*l,uint8_t dst,uint8_t seq,t425_link_packet_t*p){
    if(!l||!p) return false;
    initpkt(l,dst,T425_LACK,seq,p); l->tx_packets++; return true;
}
bool t425_reliable_on_packet(t425_reliable_link_t*l,const t425_link_packet_t*in,t425_link_packet_t*reply){
    if(!l||!in) return false;
    l->rx_packets++;
    if(in->type==T425_LACK&&l->waiting_ack&&in->seq==l->pending_seq&&in->src==l->pending_dst){
        l->waiting_ack=false; return true;
    }
    if(in->type==T425_LDATA&&reply) return t425_reliable_make_ack(l,in->src,in->seq,reply);
    return false;
}
bool t425_reliable_timeout(t425_reliable_link_t*l){
    if(!l||!l->waiting_ack) return false;
    if(l->retries<T425_RETRY_MAX){l->retries++;l->retransmissions++;return true;}
    l->waiting_ack=false;l->timeouts++;return false;
}
