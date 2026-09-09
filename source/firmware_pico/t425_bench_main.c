#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "t425_pio_phy.h"
#include "t425_phy_config.h"
#include "t425_node_config.h"

#ifndef T425_TX_PIN
#define T425_TX_PIN 2
#endif
#ifndef T425_RX_PIN
#define T425_RX_PIN 3
#endif
#ifndef T425_BAUD
#define T425_BAUD 115200u
#endif
#ifndef T425_SM_TX
#define T425_SM_TX 0
#endif
#ifndef T425_SM_RX
#define T425_SM_RX 1
#endif

static const uint8_t pattern[] = {0x55,0xAA,0x00,0xFF,0x12,0x34,0xC3,0x3C};

int main(void){
    stdio_init_all();
    sleep_ms(1500);

    t425_pio_phy_t phy;
    bool ok=t425_pio_phy_init(&phy,pio0,T425_SM_TX,T425_SM_RX,
                              T425_TX_PIN,T425_RX_PIN,T425_BAUD);
    printf("T425_BENCH V2.37 NODE=%u\r\n", (unsigned)T425_NODE_ID);
    printf("PIO=%u TX=%u RX=%u BAUD=%u PIO_CYCLES=%u SYS_HZ=%u\r\n",
           0u,T425_TX_PIN,T425_RX_PIN,T425_BAUD,T425_PHY_PIO_CYCLES_PER_BIT,phy.sys_hz);
    printf("PHY_INIT=%s\r\n",ok?"OK":"FAIL");
    if(!ok){while(1){sleep_ms(1000);}}

    uint32_t tx=0,rx=0,err=0;
    while(1){
        if(t425_pio_phy_write(&phy,pattern,sizeof(pattern))){
            tx+=sizeof(pattern);
        } else err++;
        uint8_t r[32];
        uint16_t n=t425_pio_phy_read(&phy,r,sizeof(r));
        rx+=n;
        for(uint16_t i=0;i<n;i++){
            if(r[i]!=pattern[(rx-n+i)%sizeof(pattern)]) err++;
        }
        printf("STAT TX=%lu RX=%lu ERR=%lu\r\n",
               (unsigned long)tx,(unsigned long)rx,(unsigned long)err);
        sleep_ms(100);
    }
}
