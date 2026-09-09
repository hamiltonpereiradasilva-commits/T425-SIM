#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_link_loopback.h"

typedef struct {
    t425_link_endpoint_t *peer;
    t425_link_endpoint_t *owner;
    bool drop_ack;
    unsigned data_tx;
    unsigned ack_tx;
} channel_t;

/* One-direction callback: owner sends a frame to peer. */
static bool channel_tx(uint8_t link,const uint8_t*d,uint16_t n,void*c){
    channel_t *ch=(channel_t*)c; (void)link;
    t425_packet_t p;
    if(!t425_transport_decode(d,n,&p)) return false;
    if(p.destination==T425_ACK_DEST){
        ch->ack_tx++;
        if(ch->drop_ack) return true;
    } else {
        ch->data_tx++;
    }
    return t425_link_receive(ch->peer,d,n,100);
}

int main(void){
    t425_link_endpoint_t a,b;
    channel_t a_to_b={0},b_to_a={0};
    t425_link_endpoint_init(&a,1,0,2,10,channel_tx,&a_to_b);
    t425_link_endpoint_init(&b,1,1,2,10,channel_tx,&b_to_a);
    a_to_b.peer=&b; b_to_a.peer=&a;

    t425_packet_t p={.destination=1,.source=0,.ttl=8,.length=5,.payload={'H','E','L','L','O'}};

    /* 1) DATA -> ACK -> success */
    assert(t425_link_send(&a,&p,100));
    assert(a.rl.state==T425_RL2_IDLE);
    assert(a.rl.ack_ok==1);
    assert(b.rx_data==1);
    assert(a_to_b.data_tx==1 && b_to_a.ack_tx==1);

    /* 2) Lose ACK. Sender must remain waiting. */
    b_to_a.drop_ack=true;
    assert(t425_link_send(&a,&p,200));
    assert(a.rl.state==T425_RL2_WAIT_ACK);
    assert(a.rl.pending_valid);
    uint8_t pending_seq=a.rl.pending_seq;

    /* 3) Timeout -> retry */
    t425_link_poll(&a,210);
    assert(a.rl.state==T425_RL2_RETRY);
    t425_packet_t retry; uint8_t retry_seq;
    assert(t425_rl2_take_retry(&a.rl,&retry,&retry_seq));
    assert(retry_seq==pending_seq);
    assert(retry.sequence==pending_seq);

    /* Send the cached retry manually, still with ACK suppressed. */
    uint8_t wire[T425_WIRE_MAX];
    uint16_t n=t425_transport_encode(&retry,wire,sizeof(wire));
    assert(n>0);
    assert(channel_tx(1,wire,n,&a_to_b));
    assert(a.rl.state==T425_RL2_WAIT_ACK);

    /* 4) Restore ACK and deliver matching ACK. */
    b_to_a.drop_ack=false;
    t425_packet_t ack={.destination=T425_ACK_DEST,.source=1,.ttl=1,
                       .sequence=pending_seq,.length=1,.payload={T425_ACK_PAYLOAD}};
    n=t425_transport_encode(&ack,wire,sizeof(wire)); assert(n>0);
    assert(t425_link_receive(&a,wire,n,211));
    assert(a.rl.state==T425_RL2_IDLE && a.rl.ack_ok==2);

    /* 5) CRC failure */
    wire[9]^=0x01;
    assert(!t425_link_receive(&a,wire,n,212));
    assert(a.rx_bad_crc==1);

    puts("T425 ONE-HOP DATA/ACK/RETRY: PASS");
    return 0;
}
