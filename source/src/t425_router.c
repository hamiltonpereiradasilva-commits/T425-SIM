#include "t425_router.h"
#include <string.h>

void t425_router_init(t425_router_t*r,uint8_t id){
    memset(r,0,sizeof(*r)); r->node_id=id;
}
bool t425_router_add_route(t425_router_t*r,uint8_t dst,uint8_t next,uint8_t metric){
    if(!r||r->route_count>=T425_ROUTER_MAX_ROUTES||metric==0||next==r->node_id)return false;
    for(uint8_t i=0;i<r->route_count;i++)
        if(r->routes[i].valid&&r->routes[i].dst==dst){
            if(metric<r->routes[i].metric)
                r->routes[i]=(t425_route_t){dst,next,metric,true};
            return true;
        }
    r->routes[r->route_count++]=(t425_route_t){dst,next,metric,true};
    return true;
}
bool t425_router_lookup(const t425_router_t*r,uint8_t dst,uint8_t*next){
    if(!r||!next)return false;
    if(dst==r->node_id){*next=r->node_id;return true;}
    for(uint8_t i=0;i<r->route_count;i++)
        if(r->routes[i].valid&&r->routes[i].dst==dst){*next=r->routes[i].next_hop;return true;}
    return false;
}
bool t425_router_forward(const t425_router_t*r,const t425_link_packet_t*in,t425_link_packet_t*out){
    uint8_t nh;
    if(!r||!in||!out||in->dst==r->node_id)return false;
    if(!t425_router_lookup(r,in->dst,&nh))return false;
    *out=*in;
    out->src=in->src;
    out->dst=nh;
    return true;
}
