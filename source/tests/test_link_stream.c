#include <assert.h>
#include <stdio.h>
#include "t425_link_stream.h"
int main(void){
 t425_link_packet_t a={.sof=T425_LINK_SOF,.src=0,.dst=1,.type=T425_LDATA,.seq=3,.len=3,.payload={1,2,3}},b;
 uint8_t raw[300];uint16_t n;assert(t425_link_packet_encode(&a,raw,&n));
 t425_link_stream_t s;t425_link_stream_init(&s);
 bool got=false;for(uint16_t i=0;i<n;i++)if(t425_link_stream_push(&s,raw[i],&b))got=true;
 assert(got&&b.src==0&&b.dst==1&&b.seq==3&&b.len==3);
 puts("stream parser: PASS");return 0;
}
