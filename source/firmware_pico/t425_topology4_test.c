#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_router.h"
#include "t425_topology4.h"

int main(void) {
    stdio_init_all();
    sleep_ms(800);

    t425_router_t r;
    t425_router_init(&r, T425_NODE_ID);
    t425_topology4_install(&r);

    printf("T425_TOPOLOGY V2.47 NODE=%u LINE4\r\n", r.node_id);

    for (uint8_t dst=0; dst<4; ++dst) {
        uint8_t hop=0xFF;
        t425_route_action_t a=t425_router_lookup(&r,dst,&hop);
        printf("DST=%u ACTION=%u NEXT=%u\r\n",dst,(unsigned)a,hop);
    }

    while(1) sleep_ms(1000);
}
