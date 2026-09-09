#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_interfaces.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_interface_table_t t;
    t425_interfaces_init(&t);

    /* Example P00: two independent physical links. */
    t425_interface_add(&t, 1, 0, 2, 3); /* IF0 -> P01 */
    t425_interface_add(&t, 2, 1, 4, 5); /* IF1 -> P02 */

    printf("T425_IF V2.54 NODE=P00\r\n");
    for (uint8_t i=0; i<t.count; ++i)
        printf("IF%u PEER=P%u PIO_SM=%u TX=GPIO%u RX=GPIO%u STATE=%u\r\n",
               i, t.items[i].peer_id, t.items[i].pio_sm,
               t.items[i].tx_gpio, t.items[i].rx_gpio, t.items[i].state);

    printf("ROUTE NEXT=P02 -> IF=%d\r\n", t425_interface_for_peer(&t, 2));

    t425_interface_set_state(&t, 1, T425_IF_DOWN);
    printf("IF1 DOWN -> ROUTE NEXT=P02 -> IF=%d\r\n",
           t425_interface_for_peer(&t, 2));

    t425_interface_set_state(&t, 1, T425_IF_UP);
    printf("IF1 RECOVERED -> ROUTE NEXT=P02 -> IF=%d\r\n",
           t425_interface_for_peer(&t, 2));

    while (1) sleep_ms(1000);
}
