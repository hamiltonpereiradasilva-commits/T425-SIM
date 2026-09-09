#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_flash_slots.h"

static t425_config_record_t make_cfg(uint8_t node) {
    t425_node_config_t c;
    t425_node_config_init(&c,node);
    t425_node_config_add_interface(&c,1,0,2,3);
    t425_config_record_t r;
    t425_config_build(&r,&c);
    return r;
}

int main(void) {
    stdio_init_all(); sleep_ms(700);

    t425_flash_image_t img = {0};
    t425_slot_id_t active = t425_flash_select_active(&img);
    printf("T425 FLASH SLOTS V2.59\r\n");
    printf("EMPTY ACTIVE=%u\r\n", active);

    t425_config_record_t r4=make_cfg(4);
    t425_flash_image_t next;
    t425_slot_id_t target;

    if (t425_flash_prepare_commit(&img,&r4,&next,&target)) {
        img=next;
        printf("COMMIT1 TARGET=%u GEN_A=%lu GEN_B=%lu ACTIVE=%u\r\n",
               target,(unsigned long)img.a.generation,
               (unsigned long)img.b.generation,
               t425_flash_select_active(&img));
    }

    t425_config_record_t r5=make_cfg(5);
    if (t425_flash_prepare_commit(&img,&r5,&next,&target)) {
        img=next;
        printf("COMMIT2 TARGET=%u GEN_A=%lu GEN_B=%lu ACTIVE=%u\r\n",
               target,(unsigned long)img.a.generation,
               (unsigned long)img.b.generation,
               t425_flash_select_active(&img));
    }

    /* Simulate corruption of newest slot. */
    img.b.record.config.node_id=99;
    printf("CORRUPTED_B ACTIVE=%u\r\n", t425_flash_select_active(&img));

    /* Restore a valid newer record. */
    img.b.record=r5;
    printf("RECOVERED_B ACTIVE=%u\r\n", t425_flash_select_active(&img));

    while (1) sleep_ms(1000);
}
