#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_link_packet.h"
int main(void){
 t425_link_packet_t a={.sof=T425_LINK_SOF,.src=0,.dst=1,.type=T425_LDATA,.seq=7,.len=5,.payload={10,20,30,40,50}};
 uint8_t b[300];uint16_t n=0;assert(t425_link_packet_encode(&a,b,&n));
 t425_link_packet_t c={0};assert(t425_link_packet_decode(b,n,&c));
 assert(c.src==0&&c.dst==1&&c.seq==7&&c.len==5&&memcmp(c.payload,a.payload,5)==0);
 b[6]^=1;assert(!t425_link_packet_decode(b,n,&c));
 puts("link packet encode/decode: PASS");return 0;
}
