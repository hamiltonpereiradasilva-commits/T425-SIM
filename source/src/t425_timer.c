#include "t425_timer.h"
void t425_timer_init(t425_timer_t*t){t->now=0;for(int i=0;i<T425_MAX_ALTS;i++){t->guard[i].time=0;t->guard[i].enabled=false;t->guard[i].fired=false;}}
void t425_timer_tick(t425_timer_t*t,uint32_t ticks){
    t->now += ticks;
    for(int i=0;i<T425_MAX_ALTS;i++)
        if(t->guard[i].enabled && t->now>=t->guard[i].time)t->guard[i].fired=true;
}
int t425_timer_tin(t425_timer_t*t,uint32_t i,uint32_t deadline){
    if(i>=T425_MAX_ALTS)return 0;
    t->guard[i].time=deadline;t->guard[i].enabled=true;t->guard[i].fired=(t->now>=deadline);return 1;
}
void t425_timer_enbt(t425_timer_t*t,uint32_t i){if(i<T425_MAX_ALTS)t->guard[i].enabled=false;}
bool t425_timer_ready(t425_timer_t*t,uint32_t i){return i<T425_MAX_ALTS&&t->guard[i].enabled&&t->guard[i].fired;}
