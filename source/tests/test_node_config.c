#include <assert.h>
#include <stdio.h>
#include "t425_node_config.h"
int main(void){
 t425_node_config_t c;
 t425_node_config_init(&c,7);
 assert(c.node_id==7&&c.link_baud==1000000u);
 assert(t425_node_config_add_link(&c,0,6,0,1,0,0,1));
 assert(t425_node_config_add_link(&c,1,8,2,3,0,2,3));
 assert(c.link_count==2);
 const t425_link_cfg_t*l=t425_node_config_get_link(&c,1);
 assert(l&&l->neighbor_id==8&&l->tx_pin==2&&l->rx_pin==3);
 assert(!t425_node_config_get_link(&c,2));
 puts("GENERIC NODE CONFIG: PASS");return 0;
}
