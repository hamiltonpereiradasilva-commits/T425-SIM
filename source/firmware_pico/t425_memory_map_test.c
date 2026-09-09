#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_config_address.h"
#include "t425_flash_layout.h"

int main(void) {
    stdio_init_all(); sleep_ms(700);
    uintptr_t a=t425_config_flash_base();
    uintptr_t b=t425_config_flash_end();
    printf("T425 MEMORY MAP V2.62\r\n");
    printf("CONFIG_START=0x%08lX\r\n",(unsigned long)a);
    printf("CONFIG_END=0x%08lX\r\n",(unsigned long)b);
    printf("CONFIG_SIZE=%lu\r\n",(unsigned long)(b-a));
    printf("EXPECTED_SIZE=%u\r\n",T425_FLASH_CONFIG_REGION);
    printf("ALIGNMENT=%u\r\n",
           (unsigned)((a % T425_FLASH_ERASE_SIZE)==0));
    while(1) sleep_ms(1000);
}
