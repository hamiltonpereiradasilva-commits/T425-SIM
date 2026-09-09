#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_link_health.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_link_health_t h;
    t425_link_health_init(&h, 3, 3);

    printf("T425_FAILURE V2.51 PEER=%u\r\n", h.peer);

    t425_link_health_on_ack(&h);
    printf("ACK STATE=%u TIMEOUTS=%u\r\n", h.state, h.consecutive_timeouts);

    for (int i=0; i<3; ++i) {
        t425_link_health_on_timeout(&h);
        printf("TIMEOUT #%d STATE=%u CONSEC=%u\r\n",
               i+1, h.state, h.consecutive_timeouts);
    }

    printf("ROUTE_ACTION=REQUIRE_ALTERNATIVE\r\n");

    t425_link_health_on_ack(&h);
    printf("RECOVERY ACK STATE=%u CONSEC=%u\r\n",
           h.state, h.consecutive_timeouts);

    while (1) sleep_ms(1000);
}
