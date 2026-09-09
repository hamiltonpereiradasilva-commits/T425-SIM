#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "t425_pio_phy.h"
#include "t425_node_config.h"
#include "t425_reliable_link.h"

static t425_pio_phy_t phy;
static t425_rl_t rl;
static t425_parser_t parser;
static t425_frame_t pending;

static void wire_send(const t425_frame_t*f){
    t425_pio_phy_write(&phy,(const uint8_t*)f,(uint16_t)(T425_OVERHEAD+f->len));
}

static void make_data(uint8_t seq){
    uint8_t p[32];
    for(uint8_t i=0;i<32;i++)p[i]=(uint8_t)(seq+i);
    t425_frame_build(&pending,T425_TYPE_DATA,T425_NODE_ID,T425_PEER_ID,seq,p,32);
}

static void send_ack(uint8_t dst,uint8_t seq){
    t425_frame_t a;
    t425_frame_build(&a,T425_TYPE_ACK,T425_NODE_ID,dst,seq,0,0);
    wire_send(&a);
}

static void start_new(uint32_t now){
    make_data(rl.next_tx_seq);
    wire_send(&pending);
    t425_rl_start_tx(&rl,rl.next_tx_seq,now);
}

static void retry(uint32_t now){
    wire_send(&pending);
    t425_rl_note_retransmit(&rl,now);
}

int main(void){
    stdio_init_all();
    sleep_ms(1200);

    if(!t425_pio_phy_init(&phy,pio0,0,1,T425_TX_PIN,T425_RX_PIN,T425_BAUD)){
        printf("T425_RL V2.42 NODE=%u INIT=FAIL\r\n",T425_NODE_ID);
        while(1)sleep_ms(1000);
    }

    t425_pio_phy_reset_fifos(&phy);
    t425_rl_init(&rl,50,5);
    t425_parser_init(&parser);

    printf("T425_RL V2.42 NODE=%u PEER=%u TIMEOUT=%lu MAX_RETRY=%u\r\n",
           T425_NODE_ID,T425_PEER_ID,(unsigned long)rl.timeout_ms,rl.max_retries);

    uint32_t last_report=0;

    while(1){
        uint8_t b[64];
        uint16_t n=t425_pio_phy_read(&phy,b,sizeof(b));

        for(uint16_t i=0;i<n;i++){
            t425_frame_t f;
            if(!t425_parser_push(&parser,b[i],&f))continue;
            if(f.dst!=T425_NODE_ID)continue;

            rl.rx_frames++;

            if(f.type==T425_TYPE_ACK){
                if(t425_rl_accept_ack(&rl,f.seq)){
                    rl.next_tx_seq=(uint8_t)(f.seq+1u);
                    rl.pending_valid=false;
                }
            } else if(f.type==T425_TYPE_DATA){
                bool duplicate=false;
                bool deliver=t425_rl_accept_data(&rl,f.seq,&duplicate);
                /* Valid duplicates receive ACK again, but are never delivered twice. */
                send_ack(f.src,f.seq);
                (void)deliver;
            }
        }

        uint32_t now=time_us_32()/1000u;

        if(T425_NODE_ID==0u){
            if(!rl.waiting_ack && !rl.pending_valid)
                start_new(now);

            if(t425_rl_timeout(&rl,now)){
                if(rl.retries<rl.max_retries)
                    retry(now);
                else {
                    t425_rl_note_timeout(&rl);
                    rl.next_tx_seq++;
                }
            }
        }

        if(now-last_report>=1000u){
            last_report=now;
            printf("STAT NODE=%u TX=%lu RX=%lu ACK=%lu RETX=%lu TIMEOUT=%lu "
                   "CRC=%lu FORMAT=%lu SYNCLOSS=%lu DUP=%lu OOO=%lu\r\n",
                   T425_NODE_ID,
                   (unsigned long)rl.tx_frames,
                   (unsigned long)rl.rx_frames,
                   (unsigned long)rl.acked,
                   (unsigned long)rl.retransmits,
                   (unsigned long)rl.timeouts,
                   (unsigned long)parser.crc_errors,
                   (unsigned long)parser.format_errors,
                   (unsigned long)parser.sync_losses,
                   (unsigned long)rl.duplicate_data,
                   (unsigned long)rl.out_of_order);
        }

        sleep_ms(1);
    }
}
