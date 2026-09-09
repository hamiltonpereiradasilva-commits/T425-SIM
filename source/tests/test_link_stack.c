#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_link.h"

int main(void){
 t425_link_stack_t a,b; t425_frame_t rx; uint16_t n;
 const uint8_t p[]={0x10,0x20,0x30};
 t425_link_stack_init(&a); t425_link_stack_init(&b);
 assert(t425_link_stack_prepare(&a,0,1,p,sizeof(p),&n));
 assert(t425_link_stack_receive(&b,a.tx_buf,n,&rx));
 assert(rx.src==0&&rx.dst==1&&rx.type==T425_DATA_TYPE&&rx.seq==0&&rx.len==3);
 assert(!memcmp(rx.payload,p,3));
 assert(a.reliable.tx_count==1&&b.reliable.rx_count==1&&b.frames_ok==1);
 puts("LINK STACK FRAME+CRC+SEQ RX/TX MODEL: PASS");
 return 0;
}
