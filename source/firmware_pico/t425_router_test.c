#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_node_config.h"
#include "t425_router.h"

int main(void) {
    stdio_init_all();
    sleep_ms(1000);

    t425_router_t r;
    t425_router_init(&r, T425_NODE_ID);

    /* Initial two-node topology. Extend later without changing the API. */
    if (T425_NODE_ID == 0u) {
        t425_router_set_route(&r, 1u, 1u);
    } else if (T425_NODE_ID == 1u) {
        t425_router_set_route(&r, 0u, 0u);
    }

    printf("T425_ROUTER V2.43 NODE=%u\r\n", T425_NODE_ID);

    uint8_t destinations[] = {T425_NODE_ID, 0u, 1u, 2u};
    for (unsigned i = 0; i < sizeof(destinations); ++i) {
        uint8_t hop = 0xFFu;
        t425_route_action_t a =
            t425_router_lookup(&r, destinations[i], &hop);

        printf("ROUTE DST=%u ACTION=%u HOP=%u\r\n",
               destinations[i], (unsigned)a, hop);
    }

    while (1) {
        sleep_ms(1000);
    }
}
