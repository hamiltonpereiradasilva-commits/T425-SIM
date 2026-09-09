#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_e2e.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_e2e_t src;
    t425_e2e_init(&src, 3);
    t425_e2e_start(&src, 42, 0, 3);

    uint8_t last_tx_id = 0;
    bool valid_last = false;

    printf("T425_E2E V2.49\r\n");

    /* First delivery at P03. */
    bool first = t425_e2e_destination_accept(42, &last_tx_id, &valid_last);
    printf("P03 DELIVERY TX_ID=42 ACCEPT=%u\r\n", first);

    /* Simulated lost E2E ACK: source remains pending and retries. */
    printf("SOURCE STATE BEFORE RETRY=%u ATT=%u\r\n",
           src.state, src.attempts);
    t425_e2e_retry(&src);

    /* Duplicate reaches P03. It must not reach the application twice. */
    bool second = t425_e2e_destination_accept(42, &last_tx_id, &valid_last);
    printf("P03 DUPLICATE TX_ID=42 ACCEPT=%u\r\n", second);

    /* Destination sends the E2E delivery acknowledgement again. */
    t425_e2e_on_delivery(&src, 42);
    printf("SOURCE FINAL STATE=%u ATT=%u\r\n", src.state, src.attempts);

    while (1) sleep_ms(1000);
}
