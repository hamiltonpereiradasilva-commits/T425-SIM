#include <assert.h>
#include <stdio.h>
#include "t425_runtime.h"
int main(void){
 t425_runtime_t rt; t425_node_config_t c; t425_frame_t f,o;
 t425_node_config_init(&c,0);
 t425_node_config_add_link(&c,0,1,0,1,0,0,1);
 assert(t425_runtime_init(&rt,&c,2,5));
 assert(rt.state==T425_RT_READY);
 f.src=0;f.dst=1;f.type=1;f.seq=3;f.len=2;f.payload[0]=0xAA;f.payload[1]=0x55;
 uint8_t raw[256];uint16_t n;assert(t425_frame_encode(&f,raw,sizeof(raw),&n));
 bool got=false;for(uint16_t i=0;i<n;i++)if(t425_runtime_rx_byte(&rt,0,raw[i],&o))got=true;
 assert(got&&rt.frames_rx==1&&o.seq==3&&o.payload[1]==0x55);
 assert(t425_ack_start(&rt.ack[0],3,rt.ticks));
 assert(t425_runtime_ack_received(&rt,0,3));
 assert(rt.ack[0].acked==1);
 puts("T425 NODE RUNTIME INTEGRATION: PASS");return 0;
}
