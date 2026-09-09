#include "t425_runtime.h"
#include <string.h>
bool t425_runtime_init(t425_runtime_t*rt,const t425_node_config_t*c,
                       uint8_t max,uint32_t timeout){
 if (!rt || !c) {
        return false;
    }
    memset(rt, 0, sizeof(*rt));
    rt->config = *c;
 for(uint8_t i=0;i<T425_MAX_LINKS;i++){
   t425_stream_init(&rt->rx_parser[i]);
   t425_ack_init(&rt->ack[i],max,timeout);
 }
 rt->state=T425_RT_READY;return true;
}
void t425_runtime_tick(t425_runtime_t*rt){
 if (!rt) {
        return;
    }
    rt->ticks++;
 for(uint8_t i=0;i<T425_MAX_LINKS;i++){
   if(i < rt->config.interfaces.count && rt->config.interfaces.items[i].state != T425_IF_DOWN){
     if(t425_ack_poll(&rt->ack[i],rt->ticks)){
       if(rt->ack[i].state==T425_TX_RETRY) { /* scheduler consumes retry */ }
     }
   }
 }
}
bool t425_runtime_rx_byte(t425_runtime_t*rt,uint8_t ll,uint8_t b,t425_frame_t*out){
 if(!rt||ll>=T425_MAX_LINKS || ll >= rt->config.interfaces.count ||
   rt->config.interfaces.items[ll].state == T425_IF_DOWN)return false;
 if(t425_stream_push(&rt->rx_parser[ll],b,out)){rt->frames_rx++;return true;}
 return false;
}
bool t425_runtime_ack_received(t425_runtime_t*rt,uint8_t ll,uint8_t seq){
 if(!rt||ll>=T425_MAX_LINKS)return false;
 return t425_ack_receive(&rt->ack[ll],seq);
}
