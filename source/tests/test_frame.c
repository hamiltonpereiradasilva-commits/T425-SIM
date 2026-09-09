#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_frame.h"
int main(void){
 t425_frame_t a={.src=0,.dst=1,.type=0x20,.seq=7,.len=4,.payload={1,2,3,4}},b;
 uint8_t raw[256];uint16_t n;
 assert(t425_frame_encode(&a,raw,sizeof(raw),&n));
 assert(n==12);assert(t425_frame_decode(raw,n,&b));
 assert(!memcmp(a.payload,b.payload,a.len)&&a.src==b.src&&a.dst==b.dst&&a.type==b.type&&a.seq==b.seq);
 raw[6]^=1;assert(!t425_frame_decode(raw,n,&b));
 puts("FRAME ENCODE/DECODE + CRC REJECTION: PASS");return 0;
}
