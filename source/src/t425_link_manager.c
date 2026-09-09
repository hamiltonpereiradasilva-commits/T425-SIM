#include "t425_link_manager.h"
#include <string.h>

bool t425_link_manager_init(t425_link_manager_t*m,const t425_node_config_t*c){
    if(!m||!c)return false;
    memset(m,0,sizeof(*m));m->config=*c;
    for(uint8_t i=0;i<T425_MAX_LINKS;i++){
        const t425_link_cfg_t*l=&m->config.links[i];
        if(!l->enabled)continue;
        if(!t425_pio_link_init(&m->pio_links[i],
                               l->pio_index?pio1:pio0,l->sm_tx,l->sm_rx,
                               l->tx_pin,l->rx_pin,m->config.link_baud))
            return false;
        m->active_links++;
    }
    return true;
}
bool t425_link_manager_send_byte(t425_link_manager_t*m,uint8_t ll,uint8_t b){
    if(!m||ll>=T425_MAX_LINKS||!m->config.links[ll].enabled)return false;
    if(!t425_pio_link_put(&m->pio_links[ll],b))return false;
    m->tx_bytes++;return true;
}
bool t425_link_manager_recv_byte(t425_link_manager_t*m,uint8_t ll,uint8_t*b){
    if(!m||ll>=T425_MAX_LINKS||!m->config.links[ll].enabled||!b)return false;
    if(!t425_pio_link_get(&m->pio_links[ll],b))return false;
    m->rx_bytes++;return true;
}
bool t425_link_manager_link_enabled(const t425_link_manager_t*m,uint8_t ll){
    return m&&ll<T425_MAX_LINKS&&m->config.links[ll].enabled;
}
