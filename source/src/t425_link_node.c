#include "t425_link_node.h"
#include <string.h>
void t425_link_node_init(t425_link_node_t*n,uint8_t id){
 memset(n,0,sizeof(*n));n->node_id=id;
 t425_link_stream_init(&n->stream[0]);t425_link_stream_init(&n->stream[1]);
}
bool t425_link_node_rx_byte(t425_link_node_t*n,uint8_t link,uint8_t byte,t425_link_packet_t*out){
 if(!n||link>1)return false;
 bool ok=t425_link_stream_push(&n->stream[link],byte,out);
 if(ok)n->rx_packets++;
 return ok;
}
