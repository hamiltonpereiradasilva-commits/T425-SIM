#include <stdio.h>
#include "pico/stdlib.h"
#include "t425_firmware_config.h"

int main(void){
    stdio_init_all();
    sleep_ms(1000);
    printf("\nT425-SIM RP2040 P%u v2.8\n",T425_NODE_ID);
    printf("USB CDC READY\n");
    printf("LINK PIN TX=%u RX=%u BAUD=%u\n",
           T425_NODE_ID==0?0u:0u,T425_NODE_ID==0?1u:1u,T425_LINK_BAUD);
    printf("PIO BRING-UP READY\n");
    while(true){
        int c=getchar_timeout_us(1000);
        if(c=='i'){
            printf("INFO NODE=%u BAUD=%u\n",T425_NODE_ID,T425_LINK_BAUD);
        } else if(c=='h'){
            printf("HELP: i=info h=help\n");
        }
        tight_loop_contents();
    }
}
