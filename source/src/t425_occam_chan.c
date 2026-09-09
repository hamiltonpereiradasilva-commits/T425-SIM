#include "t425_occam_chan.h"
#include <string.h>
void t425_occam_chan_init(t425_occam_chan_t*c,uint8_t src,uint8_t dst,uint8_t id){
 memset(c,0,sizeof(*c));c->src_node=src;c->dst_node=dst;c->chan_id=id;c->remote=(src!=dst);
}
bool t425_occam_encode_out(const t425_occam_chan_t*c,const uint8_t*d,uint8_t n,t425_link_packet_t*p){
 if(!c||!p||n>T425_OCCAM_MAX_VALUE||(!d&&n))return false;
 memset(p,0,sizeof(*p));p->sof=T425_LINK_SOF;p->src=c->src_node;p->dst=c->dst_node;
 p->type=T425_LOUT;p->seq=c->seq;p->len=(uint8_t)(n+1);p->payload[0]=c->chan_id;
 if(n) memcpy(&p->payload[1],d,n);
 return true;
}
bool t425_occam_decode_in(const t425_occam_chan_t*c,const t425_link_packet_t*p,t425_occam_message_t*m){
 if(!c||!p||!m||p->type!=T425_LOUT||p->dst!=c->dst_node||p->len<1)return false;
 if(p->payload[0]!=c->chan_id)return false;
 m->chan_id=p->payload[0];m->len=(uint8_t)(p->len-1);memcpy(m->value,&p->payload[1],m->len);return true;
}
