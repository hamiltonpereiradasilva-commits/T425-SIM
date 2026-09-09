#include "t425_application.h"
#include "t425_transport.h"
#include <string.h>
static bool send_wire(t425_application_t*a,uint8_t link,const t425_packet_t*p){
    uint8_t wire[T425_WIRE_MAX]; uint16_t n=t425_transport_encode(p,wire,sizeof(wire));
    return n && a->emit && a->emit(link,wire,n,a->emit_ctx);
}
void t425_app_init(t425_application_t*a,t425_router_t*r,t425_emit_fn emit,void*ctx){
    memset(a,0,sizeof(*a));a->router=r;a->emit=emit;a->emit_ctx=ctx;a->default_ttl=8;
    for(unsigned i=0;i<8;i++)t425_rl_init(&a->reliable[i],5,100);
}
bool t425_app_send(t425_application_t*a,uint8_t d,const uint8_t*data,uint8_t len,uint32_t now){
    if(!a||!a->router||len>T425_ROUTE_MAX_PAYLOAD)return false;
    t425_packet_t p={0};p.destination=d;p.source=a->router->node_id;p.ttl=a->default_ttl;
    p.sequence=a->next_sequence++;p.length=len;if(len)memcpy(p.payload,data,len);
    bool local;uint8_t link;
    if(!t425_router_process(a->router,&p,&local,&link))return false;
    if(local){a->delivered++;return true;}
    uint8_t seq;
    if(!t425_rl_begin(&a->reliable[link],now,&seq))return false;
    p.sequence=seq;
    if(!send_wire(a,link,&p)){a->reliable[link].state=T425_RL_FAILED;a->failed++;return false;}
    t425_rl_tx_result(&a->reliable[link],true);a->sent++;return true;
}
void t425_app_poll(t425_application_t*a,uint32_t now){
    if(!a)return;
    for(unsigned link=0;link<8;link++){
        t425_rl_poll(&a->reliable[link],now);
        if(t425_rl_should_retry(&a->reliable[link])){
            uint8_t seq;if(t425_rl_take_retry(&a->reliable[link],&seq)){
                (void)seq; /* retransmission frame cache is the next integration item */
            }
        }
        if(a->reliable[link].state==T425_RL_FAILED)a->failed=a->reliable[link].failures;
    }
}
