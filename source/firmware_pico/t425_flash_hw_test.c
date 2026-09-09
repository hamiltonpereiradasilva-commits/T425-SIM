#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_flash_hw.h"

int main(void) {
    stdio_init_all(); sleep_ms(700);
    printf("T425 RP2040 FLASH HW ABSTRACTION V2.60\r\n");
    printf("BOOT/COMMIT SEQUENCING ENABLED\r\n");
    printf("PHYSICAL DRIVER STATUS=NOT_BOUND\r\n");
    printf("SAFE MODE: no destructive flash operation\r\n");
    while (1) sleep_ms(1000);
}
