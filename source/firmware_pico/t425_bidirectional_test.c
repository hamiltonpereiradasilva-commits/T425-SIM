#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_router.h"
#include "t425_topology4.h"
#include "t425_e2e.h"

/*
 * v2.50: deterministic bidirectional routing matrix test.
 * This test validates the control-plane semantics without claiming physical
 * delivery until executed on four Pico.
 */
typedef struct { uint8_t src, dst, expected_first_hop; } route_case_t;

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    static const route_case_t cases[] = {
        {0,3,1}, {3,0,2}, {1,3,2}, {2,0,1},
        {0,2,1}, {3,1,2}, {1,0,0}, {2,3,3}
    };

    printf("T425_BIDIR V2.50\r\n");
    for (unsigned i=0; i<sizeof(cases)/sizeof(cases[0]); ++i) {
        t425_router_t r;
        t425_router_init(&r, cases[i].src);
        t425_topology4_install(&r);
        uint8_t hop=0xFF;
        t425_route_action_t a=t425_router_lookup(&r,cases[i].dst,&hop);
        printf("CASE %u %u->%u ACTION=%u NEXT=%u EXPECT=%u %s\r\n",
               i,cases[i].src,cases[i].dst,(unsigned)a,hop,
               cases[i].expected_first_hop,
               (a==T425_ROUTE_FORWARD && hop==cases[i].expected_first_hop)?"PASS":"FAIL");
    }
    while(1) sleep_ms(1000);
}
