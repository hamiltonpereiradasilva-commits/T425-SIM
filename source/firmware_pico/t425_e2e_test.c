#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_e2e.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_e2e_t e;
    t425_e2e_init(&e, 3);

    printf("T425_E2E V2.48\r\n");

    t425_e2e_start(&e, 42, 0, 3);
    printf("START ID=%u STATE=%u ATT=%u\r\n",
           e.tx_id, e.state, e.attempts);

    /* Local hop ACKs are deliberately not treated as final delivery. */
    printf("HOP_ACK P00-P01 => E2E_STATE=%u\r\n", e.state);
    printf("HOP_ACK P01-P02 => E2E_STATE=%u\r\n", e.state);
    printf("HOP_ACK P02-P03 => E2E_STATE=%u\r\n", e.state);

    t425_e2e_on_delivery(&e, 42);
    printf("FINAL_DELIVERY ID=%u STATE=%u\r\n", e.tx_id, e.state);

    while (1) sleep_ms(1000);
}
