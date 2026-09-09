#include "t425_link_loopback.h"
#include <string.h>

void t425_link_endpoint_init(t425_link_endpoint_t*e,uint8_t link,uint8_t node_id,
                             uint8_t max,uint32_t timeout,t425_phy_tx_fn tx,void*ctx){
 memset(e,0,sizeof(*e));e->link=link;e->node_id=node_id;e->tx=tx;e->ctx=ctx;
 t425_rl2_init(&e->rl,max,timeout);
}
bool t425_link_send(t425_link_endpoint_t*e,t425_packet_t*p,uint32_t now){
 uint8_t seq,wire[T425_WIRE_MAX];
 if(!e||!p||!t425_rl2_begin(&e->rl,p,now,&seq))return false;
 uint16_t n=t425_transport_encode(&e->rl.pending_packet,wire,sizeof(wire));
 if(!n||!e->tx||!e->tx(e->link,wire,n,now,e->ctx)){e->rl.state=T425_RL2_FAILED;e->rl.pending_valid=false;return false;}
 t425_rl2_tx_result(&e->rl,true);return true;
}
bool t425_link_receive(t425_link_endpoint_t*e,const uint8_t*b,uint16_t n,uint32_t now){
 t425_packet_t p;
 if(!e||!t425_transport_decode(b,n,&p)){if(e)e->rx_bad_crc++;return false;}
 e->rx_frames++;
 if(p.destination==T425_ACK_DEST && p.length==1 && p.payload[0]==T425_ACK_PAYLOAD){
   if(t425_rl2_ack(&e->rl,p.sequence,now)){e->rx_ack++;return true;}
   return false;
 }
 e->rx_data++;
 if(e->tx){
   t425_packet_t ack={.destination=T425_ACK_DEST,.source=e->node_id,.ttl=1,
                      .sequence=p.sequence,.length=1,.payload={T425_ACK_PAYLOAD}};
   uint8_t w[T425_WIRE_MAX];uint16_t wn=t425_transport_encode(&ack,w,sizeof(w));
   if(wn)e->tx(e->link,w,wn,now,e->ctx);
 }
 return true;
}
void t425_link_poll(t425_link_endpoint_t*e,uint32_t now){
 if(!e)return;
 t425_rl2_poll(&e->rl,now);
 if(e->rl.state==T425_RL2_RETRY){
   t425_packet_t p;uint8_t seq;
   if(t425_rl2_take_retry(&e->rl,&p,&seq)){
     uint8_t w[T425_WIRE_MAX];uint16_t n=t425_transport_encode(&p,w,sizeof(w));
     if(n&&e->tx)e->tx(e->link,w,n,now,e->ctx);
   }
 }
}
