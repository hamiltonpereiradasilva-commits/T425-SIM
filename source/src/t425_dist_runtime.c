#include "t425_dist_runtime.h"
#include <string.h>
void t425_dist_init(t425_dist_runtime_t*d,uint8_t id){
 memset(d,0,sizeof(*d));d->node_id=id;t425_remote_rdv_init(&d->remote,id);
}
bool t425_dist_bind(t425_dist_runtime_t*d,uint8_t ch,uint8_t src,uint8_t dst){
 if(!d||ch>=T425_DIST_MAX_CHANS||d->route_count>=T425_DIST_MAX_CHANS)return false;
 for(uint8_t i=0;i<d->route_count;i++)if(d->routes[i].used&&d->routes[i].chan==ch)return false;
 d->routes[d->route_count++]=(t425_route_t){src,dst,ch,true};return true;
}
bool t425_dist_send(t425_dist_runtime_t*d,uint8_t ch,const uint8_t*x,uint8_t n,t425_link_packet_t*p){
 if(!d||!p)return false;
 for(uint8_t i=0;i<d->route_count;i++)
  if(d->routes[i].used&&d->routes[i].chan==ch&&d->routes[i].src==d->node_id)
   return t425_remote_rdv_make_out(&d->remote,d->routes[i].dst,ch,x,n,p);
 return false;
}
bool t425_dist_receive(t425_dist_runtime_t*d,const t425_link_packet_t*p,t425_link_packet_t*reply){
 if(!d||!p)return false;
 return t425_remote_rdv_handle(&d->remote,p,reply);
}
