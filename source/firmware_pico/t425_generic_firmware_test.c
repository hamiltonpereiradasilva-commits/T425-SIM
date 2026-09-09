#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_node_profile.h"

static const t425_node_profile_t P00 = {
    .node_id=0, .interface_count=2,
    .interfaces={{1,0,2,3},{2,1,4,5}}
};

static const t425_node_profile_t P01 = {
    .node_id=1, .interface_count=2,
    .interfaces={{0,0,2,3},{3,1,4,5}}
};

static const t425_node_profile_t P02 = {
    .node_id=2, .interface_count=2,
    .interfaces={{0,0,4,5},{3,1,6,7}}
};

static const t425_node_profile_t P03 = {
    .node_id=3, .interface_count=2,
    .interfaces={{1,0,4,5},{2,1,6,7}}
};

static void print_profile(const t425_node_profile_t *p) {
    t425_node_config_t cfg;
    if (!t425_node_profile_apply(p, &cfg)) {
        printf("PROFILE ERROR\r\n");
        return;
    }
    printf("NODE=%u IF_COUNT=%u\r\n", cfg.node_id, cfg.interfaces.count);
    for (uint8_t i=0; i<cfg.interfaces.count; ++i)
        printf(" IF%u PEER=%u PIO_SM=%u TX=%u RX=%u\r\n",
               i, cfg.interfaces.items[i].peer_id,
               cfg.interfaces.items[i].pio_sm,
               cfg.interfaces.items[i].tx_gpio,
               cfg.interfaces.items[i].rx_gpio);
}

int main(void) {
    stdio_init_all();
    sleep_ms(700);
    printf("T425 GENERIC NODE FIRMWARE V2.56\r\n");
    print_profile(&P00);
    print_profile(&P01);
    print_profile(&P02);
    print_profile(&P03);
    printf("PROTOCOL_BINARY=COMMON\r\n");
    printf("NODE_ID_AND_PHYSICAL_MAP=CONFIGURATION\r\n");
    while (1) sleep_ms(1000);
}
