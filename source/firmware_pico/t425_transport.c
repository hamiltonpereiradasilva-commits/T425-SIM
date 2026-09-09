#include "t425_transport.h"
#include "t425_crc16.h"
#include <string.h>
uint16_t t425_transport_encode(const t425_packet_t*p,uint8_t*out,uint16_t cap){
    if(!p||!out||p->length>T425_ROUTE_MAX_PAYLOAD)return 0;
    uint16_t n=(uint16_t)(9+p->length+2);
    if(cap<n)return 0;
    out[0]=T425_WIRE_MAGIC0;out[1]=T425_WIRE_MAGIC1;out[2]=T425_WIRE_VERSION;
    out[3]=p->destination;out[4]=p->source;out[5]=p->ttl;out[6]=p->sequence;out[7]=p->length;
    out[8]=0; /* reserved */
    if(p->length)memcpy(&out[9],p->payload,p->length);
    uint16_t crc=t425_crc16_ccitt(out,9+p->length);
    out[9+p->length]=(uint8_t)(crc>>8);out[10+p->length]=(uint8_t)crc;
    return n;
}
bool t425_transport_decode(const uint8_t*b,uint16_t n,t425_packet_t*p){
    if(!b||!p||n<11||b[0]!=T425_WIRE_MAGIC0||b[1]!=T425_WIRE_MAGIC1||b[2]!=T425_WIRE_VERSION)return false;
    uint8_t l=b[7]; if(l>T425_ROUTE_MAX_PAYLOAD||n!=(uint16_t)(11+l))return false;
    uint16_t got=(uint16_t)((b[9+l]<<8)|b[10+l]);
    if(got!=t425_crc16_ccitt(b,9+l))return false;
    p->destination=b[3];p->source=b[4];p->ttl=b[5];p->sequence=b[6];p->length=l;
    if(l) memcpy(p->payload,&b[9],l);
    return true;
}
