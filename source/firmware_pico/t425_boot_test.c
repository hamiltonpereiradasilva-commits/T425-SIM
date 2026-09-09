#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_runtime_config.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_runtime_config_t rt;
    printf("T425 BOOT CONFIG V2.66\r\n");

    if (!t425_runtime_config_load(&rt)) {
        printf("STATE=RECOVERY\r\n");
    } else {
        printf("STATE=CONFIGURED\r\n");
        printf("NODE_ID=%u\r\n", rt.config.node_id);
        printf("ACTIVE_SLOT=%u\r\n", rt.active_slot);
    }

    while (1) sleep_ms(1000);
}
