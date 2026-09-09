#include <assert.h>
#include <stdio.h>
#include "t425_dist_runtime.h"
int main(void){
 t425_dist_runtime_t a,b;t425_link_packet_t p,ack;uint8_t x[]={10};
 t425_dist_init(&a,0);t425_dist_init(&b,1);
 assert(t425_dist_bind(&a,1,0,1));assert(t425_dist_bind(&b,1,0,1));
 assert(t425_dist_send(&a,1,x,1,&p));
 assert(t425_dist_receive(&b,&p,&ack));
 assert(t425_dist_receive(&a,&ack,0));
 puts("DISTRIBUTED ROUTE P00->P01: PASS");return 0;
}
