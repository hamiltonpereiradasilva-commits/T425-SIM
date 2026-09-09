#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_pio_phy.h"
#include "t425_node_config.h"
#include "t425_link_bench.h"

int main(void){
    stdio_init_all();
    sleep_ms(1200);

    t425_pio_phy_t phy;
    if(!t425_pio_phy_init(&phy,pio0,0,1,T425_TX_PIN,T425_RX_PIN,T425_BAUD)){
        printf("T425_LINK V2.38 NODE=%u INIT=FAIL\r\n",T425_NODE_ID);
        while(1)sleep_ms(1000);
    }

    t425_pio_phy_reset_fifos(&phy);
    printf("T425_LINK V2.38 NODE=%u PEER=%u BAUD=%u SYS_HZ=%u\r\n",
           T425_NODE_ID,T425_PEER_ID,T425_BAUD,phy.sys_hz);

    uint8_t seq=0;
    uint32_t tx=0,rx=0,crc_err=0,dst_err=0,seq_err=0;
    uint8_t expected=0;

    while(1){
        /* Node 0 initiates; other nodes echo valid frames. */
        if(T425_NODE_ID==0u){
            t425_frame_t f;
            uint8_t payload[T425_TEST_PAYLOAD];
            for(uint8_t i=0;i<T425_TEST_PAYLOAD;i++) payload[i]=(uint8_t)(seq+i);
            if(t425_frame_build(&f,0,T425_PEER_ID,seq,payload,T425_TEST_PAYLOAD)){
                t425_pio_phy_write(&phy,(const uint8_t*)&f,(uint16_t)(8+f.len));
                tx++;
                seq++;
            }
        }

        uint8_t buf[96];
        uint16_t n=t425_pio_phy_read(&phy,buf,sizeof(buf));
        if(n>=8u){
            t425_frame_t f;
            memcpy(&f,buf,sizeof(f));
            if(!t425_frame_check(&f)) crc_err++;
            else if(f.dst!=T425_NODE_ID) dst_err++;
            else {
                rx++;
                if(T425_NODE_ID!=0u){
                    /* Echo the complete valid frame back to its source. */
                    f.dst=f.src; f.src=T425_NODE_ID;
                    f.crc16=t425_crc16_ccitt((const uint8_t*)&f,
                                             (uint16_t)(6u+f.len));
                    t425_pio_phy_write(&phy,(const uint8_t*)&f,
                                       (uint16_t)(8+f.len));
                } else {
                    if(f.seq!=expected) seq_err++;
                    expected=(uint8_t)(f.seq+1u);
                }
            }
        }

        if((tx+rx+crc_err+dst_err+seq_err)%100u==0u)
            printf("STAT NODE=%u TX=%lu RX=%lu CRC=%lu DST=%lu SEQ=%lu\r\n",
                   T425_NODE_ID,(unsigned long)tx,(unsigned long)rx,
                   (unsigned long)crc_err,(unsigned long)dst_err,
                   (unsigned long)seq_err);

        sleep_ms(1);
    }
}
