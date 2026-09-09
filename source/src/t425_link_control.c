#include "t425_link_control.h"
#include <string.h>
static void base(t425_link_control_t*c,uint8_t dst,uint8_t type,uint8_t seq,t425_link_packet_t*p){
    memset(p,0,sizeof(*p));
    p->sof=T425_LINK_SOF; p->src=c->node_id; p->dst=dst; p->type=type; p->seq=seq;
}
void t425_link_control_init(t425_link_control_t*c,uint8_t id){
    memset(c,0,sizeof(*c)); c->node_id=id;
}
bool t425_make_ping(t425_link_control_t*c,uint8_t dst,t425_link_packet_t*p){
    if(!c||!p) return false;
    base(c,dst,T425_LPING,c->seq++,p); c->tx++; return true;
}
bool t425_make_pong(t425_link_control_t*c,uint8_t dst,uint8_t seq,t425_link_packet_t*p){
    if(!c||!p) return false;
    base(c,dst,T425_LPONG,seq,p); c->tx++; return true;
}
bool t425_make_data(t425_link_control_t*c,uint8_t dst,const uint8_t*d,uint8_t n,t425_link_packet_t*p){
    if(!c||!p||n>T425_LINK_MAX_PAYLOAD||(!d&&n)) return false;
    base(c,dst,T425_LDATA,c->seq++,p); p->len=n;
    if(n) memcpy(p->payload,d,n);
    c->tx++; return true;
}
bool t425_handle_packet(t425_link_control_t*c,const t425_link_packet_t*in,t425_link_packet_t*reply){
    if(!c||!in) return false;
    c->rx++;
    if(in->type==T425_LPING&&reply) return t425_make_pong(c,in->src,in->seq,reply);
    return false;
}
