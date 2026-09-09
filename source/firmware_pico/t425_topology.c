#include "t425_topology.h"
void t425_topology_linear(t425_router_t*r){
    uint8_t id=r->node_id;
    if(id>0)t425_router_add_route(r,(uint8_t)(id-1),(uint8_t)(id-1),0,1);
    if(id<3)t425_router_add_route(r,(uint8_t)(id+1),(uint8_t)(id+1),1,1);
    if(id==0){t425_router_add_route(r,2,1,1,2);t425_router_add_route(r,3,1,1,3);}
    if(id==1){t425_router_add_route(r,3,2,1,2);}
    if(id==2){t425_router_add_route(r,0,1,0,2);}
    if(id==3){t425_router_add_route(r,0,2,0,3);t425_router_add_route(r,1,2,0,2);}
}
