#include "t425_link_packet.h"
static uint8_t add(const uint8_t*p,uint16_t n){uint8_t c=0;for(uint16_t i=0;i<n;i++)c=(uint8_t)(c+p[i]);return c;}
uint8_t t425_link_crc(const t425_link_packet_t*p){
 uint8_t h[6]={p->sof,p->src,p->dst,p->type,p->seq,p->len};
 uint8_t c=add(h,6);return (uint8_t)(c+add(p->payload,p->len));
}
bool t425_link_packet_encode(const t425_link_packet_t*p,uint8_t*out,uint16_t*len){
 if(!p||!out||!len||p->len>T425_LINK_MAX_PAYLOAD)return false;
 out[0]=T425_LINK_SOF;out[1]=p->src;out[2]=p->dst;out[3]=p->type;out[4]=p->seq;out[5]=p->len;
 for(uint16_t i=0;i<p->len;i++)out[6+i]=p->payload[i];
 out[6+p->len]=t425_link_crc(p);*len=(uint16_t)(7+p->len);return true;
}
bool t425_link_packet_decode(const uint8_t*in,uint16_t len,t425_link_packet_t*p){
 if(!in||!p||len<7||len>(7+T425_LINK_MAX_PAYLOAD)||in[0]!=T425_LINK_SOF)return false;
 p->sof=in[0];p->src=in[1];p->dst=in[2];p->type=in[3];p->seq=in[4];p->len=in[5];
 if((uint16_t)(7+p->len)!=len)return false;
 for(uint16_t i=0;i<p->len;i++)p->payload[i]=in[6+i];
 p->crc=in[6+p->len];return p->crc==t425_link_crc(p);
}
