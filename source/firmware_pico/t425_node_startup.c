#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_runtime_config.h"

/*
 * Canonical startup sequence for one firmware / N nodes.
 * Interface initialization and Router startup are intentionally represented
 * as integration points so they cannot silently start with an invalid NODE_ID.
 */
int t425_node_startup(void) {
    t425_runtime_config_t rt;

    if (!t425_runtime_config_load(&rt)) {
        printf("T425 BOOT: RECOVERY\r\n");
        return -1;
    }

    printf("T425 BOOT: CONFIGURED\r\n");
    printf("NODE_ID=%u\r\n", rt.config.node_id);
    printf("ACTIVE_SLOT=%u\r\n", rt.active_slot);

    /* next: initialize interfaces -> start router */
    return 0;
}
