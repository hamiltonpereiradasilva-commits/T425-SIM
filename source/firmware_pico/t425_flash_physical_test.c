#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_flash_rp2040_real.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);
    printf("T425 FLASH PHYSICAL BINDING V2.63\r\n");
    printf("ACTIVE-SLOT-PROTECTION=ON\r\n");
    printf("WRITE-VERIFY-REQUIRED=ON\r\n");
    printf("PHYSICAL SDK BINDING=PENDING\r\n");
    printf("NO DESTRUCTIVE OPERATION PERFORMED\r\n");
    while (1) sleep_ms(1000);
}
