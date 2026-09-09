#include <assert.h>
#include <stdio.h>
#include "t425_node_config.h"
int main(void){
 t425_node_config_t a,b;
 t425_node_config_init(&a,0);
 t425_node_config_add_link(&a,0,1,0,1,0,0,1);
 t425_node_config_init(&b,1);
 t425_node_config_add_link(&b,0,0,0,1,0,0,1);
 t425_node_config_add_link(&b,1,2,2,3,0,2,3);
 assert(a.node_id==0&&a.link_count==1);
 assert(b.node_id==1&&b.link_count==2);
 assert(b.links[0].neighbor_id==0&&b.links[1].neighbor_id==2);
 puts("MULTI-LINK CONFIG: PASS"); return 0;
}
