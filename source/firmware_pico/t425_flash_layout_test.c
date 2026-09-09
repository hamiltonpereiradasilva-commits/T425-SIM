#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_flash_layout.h"

int main(void) {
    stdio_init_all(); sleep_ms(700);
    t425_flash_layout_t l = {
        .slot_a_offset=0x0000,
        .slot_b_offset=0x1000,
        .slot_size=T425_FLASH_SLOT_SIZE
    };
    printf("T425 FLASH LAYOUT V2.61\r\n");
    printf("SLOT_SIZE=%u ERASE=%u PROGRAM=%u\r\n",
           T425_FLASH_SLOT_SIZE, T425_FLASH_ERASE_SIZE,
           T425_FLASH_PROGRAM_SIZE);
    printf("ALIGNMENT_VALID=%u\r\n", t425_flash_layout_validate(&l));
    while (1) sleep_ms(1000);
}
