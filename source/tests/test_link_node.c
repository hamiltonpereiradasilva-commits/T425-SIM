#include <assert.h>
#include <stdio.h>
#include "t425_link_node.h"
int main(void){
 t425_link_node_t n;t425_link_node_init(&n,2);
 t425_link_packet_t p={.sof=T425_LINK_SOF,.src=0,.dst=2,.type=T425_LPING,.seq=9,.len=0},q;
 uint8_t b[300];uint16_t z;assert(t425_link_packet_encode(&p,b,&z));
 bool ok=false;for(uint16_t i=0;i<z;i++)ok|=t425_link_node_rx_byte(&n,0,b[i],&q);
 assert(ok&&q.type==T425_LPING&&q.dst==2&&n.rx_packets==1);
 puts("link node RX: PASS");return 0;
}
