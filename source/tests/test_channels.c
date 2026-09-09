#include <assert.h>
#include <stdio.h>
#include "t425_channels.h"
int main(void){
    t425_channel_t c[T425_CHANNELS]; t425_channels_init(c); uint32_t x=0;
    assert(c[0].state==CH_FREE);
    assert(!t425_chan_out(&c[0],1,42)); assert(c[0].state==CH_WAIT_OUT);
    assert(t425_chan_in(&c[0],2,&x)); assert(x==42); assert(c[0].state==CH_READY);
    t425_channels_init(c);
    assert(!t425_chan_in(&c[1],2,&x)); assert(c[1].state==CH_WAIT_IN);
    assert(t425_chan_out(&c[1],1,99)); assert(c[1].value==99); assert(c[1].state==CH_READY);
    puts("channel rendezvous: PASS"); return 0;
}
