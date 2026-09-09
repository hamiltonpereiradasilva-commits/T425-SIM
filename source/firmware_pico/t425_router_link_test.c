#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_router_link.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_router_link_t x;
    t425_router_link_init(&x, 0, 3);

    /* P00 physical interfaces */
    t425_interface_add(&x.interfaces, 1, 0, 2, 3); /* IF0 -> P01 */
    t425_interface_add(&x.interfaces, 2, 1, 4, 5); /* IF1 -> P02 */

    /* Primary P01 and alternate P02 */
    t425_router_link_add_route(&x, 1, 2);
    t425_router_link_add_route(&x, 2, 2);

    bool up[4] = {true, true, true, true};
    uint8_t next = 0xFF;
    int if_id = -1;

    printf("T425 ROUTER+IF+LINK V2.55 NODE=P00 DST=P03\r\n");

    if (t425_router_link_resolve(&x, up, 4, &next, &if_id))
        printf("NORMAL NEXT=P%u IF=%d\r\n", next, if_id);

    t425_interface_set_state(&x.interfaces, 0, T425_IF_DOWN);
    if (t425_router_link_resolve(&x, up, 4, &next, &if_id))
        printf("FAILOVER NEXT=P%u IF=%d\r\n", next, if_id);
    else
        printf("FAILOVER NO_USABLE_ROUTE\r\n");

    t425_interface_set_state(&x.interfaces, 1, T425_IF_DOWN);
    if (t425_router_link_resolve(&x, up, 4, &next, &if_id))
        printf("UNEXPECTED NEXT=P%u IF=%d\r\n", next, if_id);
    else
        printf("BOTH_INTERFACES_DOWN NO_USABLE_ROUTE\r\n");

    while (1) sleep_ms(1000);
}
