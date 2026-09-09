#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_link_control.h"
int main(void){
 t425_link_control_t a,b;t425_link_control_init(&a,0);t425_link_control_init(&b,1);
 t425_link_packet_t p,r,d;uint8_t x[]={10,20,30};
 assert(t425_make_ping(&a,1,&p));assert(p.type==T425_LPING&&p.dst==1&&p.seq==0);
 assert(t425_handle_packet(&b,&p,&r));assert(r.type==T425_LPONG&&r.dst==0&&r.seq==0);
 assert(t425_make_data(&a,1,x,3,&d));assert(d.type==T425_LDATA&&d.len==3&&!memcmp(d.payload,x,3));
 puts("PING/PONG/DATA control: PASS");return 0;
}
