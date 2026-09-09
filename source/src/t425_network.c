#include "t425_network.h"
uint8_t t425_net_next(uint8_t node,uint8_t link){
    if(link==0)return (uint8_t)((node+T425_NET_NODES-1)%T425_NET_NODES);
    if(link==1)return (uint8_t)((node+1)%T425_NET_NODES);
    return 0xFF;
}
void t425_net_ring_defaults(uint8_t node,uint8_t*left,uint8_t*right){
    if(left)*left=t425_net_next(node,0);
    if(right)*right=t425_net_next(node,1);
}
