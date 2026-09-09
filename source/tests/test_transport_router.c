#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_transport.h"
#include "t425_router.h"
#include "t425_topology.h"
int main(void){
    t425_packet_t p={.destination=3,.source=0,.ttl=8,.sequence=42,.length=5,.payload={'H','E','L','L','O'}};
    uint8_t b[T425_WIRE_MAX];t425_packet_t q;assert(t425_transport_encode(&p,b,sizeof(b))>0);
    assert(t425_transport_decode(b,sizeof(b),&q)==false); /* exact length required */
    uint16_t n=t425_transport_encode(&p,b,sizeof(b));
    assert(t425_transport_decode(b,n,&q));assert(q.destination==3&&q.source==0&&q.sequence==42&&q.length==5);
    b[9]^=1;assert(!t425_transport_decode(b,n,&q));b[9]^=1;
    t425_router_t r;bool local;uint8_t link;
    t425_router_init(&r,0);t425_topology_linear(&r);
    assert(t425_router_process(&r,&q,&local,&link));assert(!local&&link==1&&q.ttl==7);
    t425_router_init(&r,1);t425_topology_linear(&r);
    assert(t425_router_process(&r,&q,&local,&link));assert(!local&&link==1&&q.ttl==6);
    t425_router_init(&r,2);t425_topology_linear(&r);
    assert(t425_router_process(&r,&q,&local,&link));assert(!local&&link==1&&q.ttl==5);
    t425_router_init(&r,3);t425_topology_linear(&r);
    assert(t425_router_process(&r,&q,&local,&link));assert(local&&q.ttl==5);
    puts("T425 E2E TRANSPORT/ROUTER: PASS");return 0;
}
