#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "t425_route_candidates.h"
#include "t425_redundant_topology4.h"

static void choose(const char *tag, const bool up[4],
                   const t425_route_set_t *r) {
    uint8_t hop = 0xFF;
    if (t425_route_select(r, up, 4, &hop))
        printf("%s NEXT=%u\r\n", tag, hop);
    else
        printf("%s NO_ROUTE\r\n", tag);
}

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_route_set_t r;
    t425_redundant_routes_init(0, &r, 3);

    bool up[4] = {true, true, true, true};

    printf("T425_REDUNDANT V2.53 P00_TO_P03\r\n");

    choose("NORMAL", up, &r);

    /* Primary P00->P01 fails. P00 must select P02. */
    up[1] = false;
    choose("PRIMARY_DOWN", up, &r);

    /* Both first hops fail: no route is possible. */
    up[2] = false;
    choose("BOTH_DOWN", up, &r);

    /* Primary recovers. */
    up[1] = true;
    choose("PRIMARY_RECOVERED", up, &r);

    while (1) sleep_ms(1000);
}
