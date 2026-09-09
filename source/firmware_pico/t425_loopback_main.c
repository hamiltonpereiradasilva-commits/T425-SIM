#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "t425_pio_phy.h"

#define TX_PIN 2
#define RX_PIN 3
#define BAUD 115200u
#define SM_TX 0
#define SM_RX 1

static const uint8_t pattern[] = {0x55,0xAA,0x00,0xFF,0x12,0x34,0xC3,0x3C};

int main(void){
    stdio_init_all();
    sleep_ms(1500);
    t425_pio_phy_t phy;
    bool ok=t425_pio_phy_init(&phy,pio0,SM_TX,SM_RX,TX_PIN,RX_PIN,BAUD);
    printf("T425_LOOPBACK V2.35\r\n");
    printf("TX=%u RX=%u BAUD=%u SYS_HZ=%lu\r\n",
           TX_PIN,RX_PIN,BAUD,(unsigned long)phy.sys_hz);
    printf("PHY_INIT=%s\r\n",ok?"OK":"FAIL");
    if(!ok) while(1) sleep_ms(1000);

    uint32_t tx=0,rx=0,err=0,frames=0;
    while(1){
        t425_pio_phy_write(&phy,pattern,sizeof(pattern));
        tx+=sizeof(pattern);
        sleep_us(200);
        uint8_t r[64];
        uint16_t n=t425_pio_phy_read(&phy,r,sizeof(r));
        for(uint16_t i=0;i<n;i++){
            uint8_t expected=pattern[(rx+i)%sizeof(pattern)];
            if(r[i]!=expected) err++;
        }
        rx+=n; frames++;
        printf("STAT FRAME=%lu TX=%lu RX=%lu ERR=%lu\r\n",
               (unsigned long)frames,(unsigned long)tx,
               (unsigned long)rx,(unsigned long)err);
        sleep_ms(20);
    }
}
