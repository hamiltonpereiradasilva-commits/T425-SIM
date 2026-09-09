#include "t425_links.h"
static t425_link_stats_t st[T425_LINKS];
void t425_links_init(void){for(int i=0;i<T425_LINKS;i++)st[i]=(t425_link_stats_t){0};}
bool t425_link_send(uint8_t l,const uint8_t*d,uint16_t n){
    if(l>=T425_LINKS||!d||!n||n>1024){if(l<T425_LINKS)st[l].tx_errors++;return false;}
    st[l].tx_packets++;return true; /* physical INMOS codec is next milestone */
}
bool t425_link_receive(uint8_t l,uint8_t*d,uint16_t*n){(void)d;(void)n;if(l>=T425_LINKS)return false;return false;}
void t425_link_get_stats(uint8_t l,t425_link_stats_t*s){if(s&&l<T425_LINKS)*s=st[l];}
