#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_node_config.h"
#include "t425_config_store.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);

    t425_node_config_t cfg;
    t425_node_config_init(&cfg, 4);
    t425_node_config_add_interface(&cfg, 1, 0, 2, 3);
    t425_node_config_add_interface(&cfg, 2, 1, 4, 5);

    t425_config_record_t rec;
    t425_config_build(&rec, &cfg);

    printf("T425 CONFIG V2.57\r\n");
    printf("BUILD NODE=%u IF_COUNT=%u CRC=0x%08lX\r\n",
           rec.config.node_id, rec.config.interfaces.count,
           (unsigned long)rec.crc32);

    printf("VALID=%u\r\n", t425_config_validate(&rec));

    rec.config.node_id = 9;
    printf("AFTER_MUTATION_VALID=%u\r\n", t425_config_validate(&rec));

    /* Rebuild is required after a legitimate configuration change. */
    t425_config_build(&rec, &cfg);
    printf("REBUILT_VALID=%u NODE=%u\r\n",
           t425_config_validate(&rec), rec.config.node_id);

    while (1) sleep_ms(1000);
}
