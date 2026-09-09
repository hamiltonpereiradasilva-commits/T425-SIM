#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_build_integration.h"

int main(void) {
    stdio_init_all();
    sleep_ms(700);
    printf("T425 BUILD SELF-CHECK V2.64\r\n");
    printf("FLASH_TOTAL=%lu\r\n",(unsigned long)T425_FLASH_TOTAL_BYTES);
    printf("APP_FLASH=%lu\r\n",(unsigned long)T425_APP_FLASH_BYTES);
    printf("CONFIG_RESERVED=%u\r\n",T425_CONFIG_BYTES);
    printf("CAPACITY_OK=%u\r\n",
           (unsigned)((T425_APP_FLASH_BYTES+T425_CONFIG_BYTES)
                      <= T425_FLASH_TOTAL_BYTES));
    while (1) sleep_ms(1000);
}
