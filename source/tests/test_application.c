#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_application.h"
#include "t425_topology.h"
static unsigned calls=0,last_link=0,last_len=0;
static bool emit(uint8_t l,const uint8_t*d,uint16_t n,void*c){
    (void)d;(void)c;calls++;last_link=l;last_len=n;return true;
}
int main(void){
 t425_router_t r;t425_application_t a;
 t425_router_init(&r,0);t425_topology_linear(&r);t425_app_init(&a,&r,emit,0);
 const uint8_t msg[]="HELLO";
 assert(t425_app_send(&a,3,msg,5,100));
 assert(calls==1&&last_link==1&&last_len==16&&a.sent==1);
 assert(!t425_app_send(&a,99,msg,5,100));
 assert(t425_app_send(&a,0,msg,5,100));
 assert(a.delivered==1);
 puts("T425 APPLICATION→ROUTER→RELIABLE: PASS");return 0;
}
