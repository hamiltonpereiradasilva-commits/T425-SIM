#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_runtime_state.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_runtime_state_info_t info;
    if (!t425_runtime_init(&info)) {
        printf("T425 STARTUP V2.67\r\n");
        printf("STATE=RECOVERY\r\n");
        printf("ROUTER=STOPPED\r\n");
        while (1) sleep_ms(1000);
    }

    printf("T425 STARTUP V2.67\r\n");
    printf("STATE=ONLINE\r\n");
    printf("NODE_ID=%u\r\n", info.node_id);
    printf("ACTIVE_SLOT=%u\r\n", info.active_slot);
    printf("INTERFACES_READY=%u\r\n",
           (unsigned)t425_runtime_interfaces_ready());
    printf("ROUTER_READY=%u\r\n",
           (unsigned)t425_runtime_router_ready());

    while (1) sleep_ms(1000);
}
