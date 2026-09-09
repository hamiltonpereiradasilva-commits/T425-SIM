#include "t425_alt.h"
int t425_alt_select(t425_alt_guard_t*g,uint16_t n,t425_channel_t*c,t425_timer_t*t){
    for(uint16_t i=0;i<n;i++){
        if(!g[i].enabled) continue;
        if(g[i].kind==ALT_TIMER && t425_timer_ready(t,g[i].timer_guard)) return (int)i;
        if(g[i].kind==ALT_CHANNEL && g[i].channel<T425_CHANNELS &&
           c[g[i].channel].state==CH_WAIT_OUT) return (int)i;
    }
    return -1;
}
