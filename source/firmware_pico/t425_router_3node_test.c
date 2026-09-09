#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_forward.h"
#include "t425_router.h"

static void print_route(t425_router_t*r,uint8_t dst){
    uint8_t hop=0xFFu;
    t425_route_action_t a=t425_router_lookup(r,dst,&hop);
    printf("NODE=%u DST=%u ACTION=%u HOP=%u\r\n",
           r->node_id,dst,(unsigned)a,hop);
}

int main(void){
    stdio_init_all();
    sleep_ms(1000);

    t425_router_t p0,p1,p2;
    t425_router_init(&p0,0);
    t425_router_init(&p1,1);
    t425_router_init(&p2,2);

    /* Three-node line: 0 -> 1 -> 2 and reverse. */
    t425_router_set_route(&p0,2,1);
    t425_router_set_route(&p1,0,0);
    t425_router_set_route(&p1,2,2);
    t425_router_set_route(&p2,0,1);

    t425_forward_hdr_t h;
    if(!t425_forward_build(&h,0,1,2,4,8)){
        printf("FORWARD BUILD FAIL\r\n");
        while(1)sleep_ms(1000);
    }

    printf("T425_ROUTER V2.44 3NODE\r\n");
    printf("START SRC=%u NEXT=%u FINAL=%u TTL=%u\r\n",
           h.src,h.dst,h.final_dst,h.ttl);

    uint8_t hop;
    if(t425_router_lookup(&p0,2,&hop)==T425_ROUTE_FORWARD){
        printf("P0 FORWARD HOP=%u\r\n",hop);
        t425_forward_advance(&h,hop);
    }

    if(t425_router_lookup(&p1,2,&hop)==T425_ROUTE_FORWARD){
        printf("P1 FORWARD HOP=%u\r\n",hop);
        t425_forward_advance(&h,hop);
    }

    if(t425_router_lookup(&p2,2,&hop)==T425_ROUTE_LOCAL_DELIVER){
        printf("P2 LOCAL_DELIVER HOPS=%u TTL=%u\r\n",
               h.hop_count,h.ttl);
    }

    while(1)sleep_ms(1000);
}
