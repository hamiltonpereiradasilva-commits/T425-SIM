#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "t425_link_loopback.h"

typedef struct { t425_link_endpoint_t *peer; bool drop_ack; uint32_t now; unsigned data_tx, ack_tx; } channel_t;

static bool channel_tx(uint8_t link,const uint8_t*d,uint16_t n,uint32_t now,void*c){
    channel_t *ch=(channel_t*)c; (void)link;
    t425_packet_t p;
    if(!t425_transport_decode(d,n,&p)) return false;
    ch->now=now;
    if(p.destination==T425_ACK_DEST){
        ch->ack_tx++;
        if(ch->drop_ack) return true;
    } else ch->data_tx++;
    return t425_link_receive(ch->peer,d,n,now);
}

int main(void){
    t425_link_endpoint_t a,b;
    channel_t a2b={0},b2a={0};
    t425_link_endpoint_init(&a,1,0,2,10,channel_tx,&a2b);
    t425_link_endpoint_init(&b,1,1,2,10,channel_tx,&b2a);
    a2b.peer=&b; b2a.peer=&a;

    t425_packet_t p={.destination=1,.source=0,.ttl=8,.length=5,.payload={'H','E','L','L','O'}};

    /* DATA -> ACK at t=100 */
    assert(t425_link_send(&a,&p,100));
    assert(a.rl.state==T425_RL2_IDLE && a.rl.ack_ok==1);
    assert(b.rx_data==1 && a2b.data_tx==1 && b2a.ack_tx==1);

    /* Drop ACK at t=200; sender must remain WAIT_ACK. */
    b2a.drop_ack=true;
    assert(t425_link_send(&a,&p,200));
    assert(a.rl.state==T425_RL2_WAIT_ACK);
    uint8_t pending_seq=a.rl.pending_seq;

    /* Poll before deadline: no retry. */
    t425_link_poll(&a,209);
    assert(a.rl.state==T425_RL2_WAIT_ACK);

    /* Poll exactly at deadline: retry. */
    t425_link_poll(&a,210);
    assert(a.rl.state==T425_RL2_RETRY);

    t425_packet_t retry; uint8_t retry_seq;
    assert(t425_rl2_take_retry(&a.rl,&retry,&retry_seq));
    assert(retry_seq==pending_seq && retry.sequence==pending_seq);

    /* Retry is transmitted at t=210, ACK still lost. */
    uint8_t wire[T425_WIRE_MAX];
    uint16_t n=t425_transport_encode(&retry,wire,sizeof(wire));
    assert(n>0);
    assert(channel_tx(1,wire,n,210,&a2b));
    assert(a.rl.state==T425_RL2_WAIT_ACK);

    /* Restore ACK and deliver it at t=211. */
    b2a.drop_ack=false;
    t425_packet_t ack={.destination=T425_ACK_DEST,.source=1,.ttl=1,
                       .sequence=pending_seq,.length=1,.payload={T425_ACK_PAYLOAD}};
    n=t425_transport_encode(&ack,wire,sizeof(wire)); assert(n>0);
    assert(t425_link_receive(&a,wire,n,211));
    assert(a.rl.state==T425_RL2_IDLE && a.rl.ack_ok==2);

    /* Corrupt CRC and reject at t=212. */
    wire[n-1]^=1;
    assert(!t425_link_receive(&a,wire,n,212));
    assert(a.rx_bad_crc==1);

    puts("T425 ONE-HOP TIMED DATA/ACK/RETRY: PASS");
    return 0;
}
