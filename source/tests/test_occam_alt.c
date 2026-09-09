#include <assert.h>
#include <stdio.h>
#include "t425_occam.h"
#include "t425_alt.h"
int main(void){
    static const t425_occam_instr_t p[]={{OOUT,0,0,10},{OOUT,0,0,20},{OEND,0,0,0}};
    t425_occam_program_t prog={"sender",p,3};
    assert(t425_occam_validate(&prog));
    t425_channel_t ch[T425_CHANNELS]; t425_channels_init(ch);
    t425_timer_t tm; t425_timer_init(&tm);
    t425_alt_guard_t g[2]={{ALT_CHANNEL,0,0,true},{ALT_TIMER,0,0,true}};
    assert(t425_timer_tin(&tm,0,10));
    assert(t425_alt_select(g,2,ch,&tm)==-1);
    assert(!t425_chan_out(&ch[0],1,77));
    assert(t425_alt_select(g,2,ch,&tm)==0);
    t425_timer_tick(&tm,10);
    assert(t425_alt_select(g,2,ch,&tm)==0); /* channel remains ready first */
    g[0].enabled=false;
    assert(t425_alt_select(g,2,ch,&tm)==1);
    puts("OCCAM IR + ALT selection: PASS");
    return 0;
}
