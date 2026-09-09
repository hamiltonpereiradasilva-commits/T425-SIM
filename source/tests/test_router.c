#include <assert.h>
#include <stdio.h>
#include "t425_router.h"
#include "t425_topology.h"
int main(void){
 t425_router_t r; t425_packet_t p={.destination=3,.source=0,.ttl=8,.sequence=1,.length=4,.payload={'T','4','2','5'}};
 bool local; uint8_t link;
 t425_router_init(&r,1); t425_topology_linear(&r);
 assert(t425_router_process(&r,&p,&local,&link)); assert(!local&&link==1&&p.ttl==7);
 p.destination=1;p.ttl=7;assert(t425_router_process(&r,&p,&local,&link));assert(local);
 p.destination=3;p.ttl=0;assert(!t425_router_process(&r,&p,&local,&link));
 p.destination=99;p.ttl=3;assert(!t425_router_process(&r,&p,&local,&link));
 puts("T425 ROUTER: PASS");return 0;
}
