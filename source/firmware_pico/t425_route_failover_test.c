#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "t425_route_candidates.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_route_set_t r;
    t425_route_set_init(&r, 3);

    /* Example partial-mesh destination 3:
       primary next-hop=2, alternative next-hop=1. */
    t425_route_add(&r, 2, 2);
    t425_route_add(&r, 1, 3);

    bool peer_up[4] = { true, true, true, true };
    uint8_t hop=0xFF;

    printf("T425_ROUTE V2.52 DEST=3\r\n");

    t425_route_select(&r, peer_up, 4, &hop);
    printf("NORMAL NEXT=%u\r\n", hop);

    peer_up[2] = false;
    if (t425_route_select(&r, peer_up, 4, &hop))
        printf("FAILOVER P02_DOWN NEXT=%u\r\n", hop);
    else
        printf("NO_ROUTE\r\n");

    peer_up[1] = false;
    if (t425_route_select(&r, peer_up, 4, &hop))
        printf("UNEXPECTED NEXT=%u\r\n", hop);
    else
        printf("BOTH_DOWN NO_ROUTE\r\n");

    peer_up[2] = true;
    if (t425_route_select(&r, peer_up, 4, &hop))
        printf("RECOVERY NEXT=%u\r\n", hop);

    while (1) sleep_ms(1000);
}
