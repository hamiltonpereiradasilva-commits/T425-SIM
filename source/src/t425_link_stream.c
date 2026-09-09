#include "t425_link_stream.h"
#include <string.h>
void t425_link_stream_init(t425_link_stream_t*s){memset(s,0,sizeof(*s));}
bool t425_link_stream_push(t425_link_stream_t*s,uint8_t b,t425_link_packet_t*out){
 if(!s||!out)return false;
 if(!s->active){
  if(b!=T425_LINK_SOF)return false;
  s->active=true;s->used=1;s->raw[0]=b;return false;
 }
 if(s->used>=sizeof(s->raw)){t425_link_stream_init(s);return false;}
 s->raw[s->used++]=b;
 if(s->used==6 && s->raw[5]>T425_LINK_MAX_PAYLOAD){t425_link_stream_init(s);return false;}
 if(s->used>=7 && s->used==(uint16_t)(7+s->raw[5])){
  bool ok=t425_link_packet_decode(s->raw,s->used,out);
  t425_link_stream_init(s);return ok;
 }
 return false;
}
