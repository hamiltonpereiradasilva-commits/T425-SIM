#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_occam_runtime.h"
int main(void){
 t425_occam_runtime_t r;t425_occam_message_t m;uint8_t c;uint8_t x[]={42};
 t425_occam_runtime_init(&r);
 int p0=t425_occam_proc_create(&r,1),p1=t425_occam_proc_create(&r,2);
 assert(p0==0&&p1==1);assert(t425_occam_chan_create(&r,&c)&&c==0);
 assert(!t425_occam_in(&r,p1,c,&m));assert(r.proc[p1].state==T425_PROC_BLOCKED_IN);
 assert(t425_occam_out(&r,p0,c,x,1));assert(r.proc[p1].state==T425_PROC_READY);
 assert(t425_occam_in(&r,p1,c,&m));assert(m.len==1&&m.value[0]==42);
 t425_occam_schedule(&r);assert(r.current==1&&r.proc[1].state==T425_PROC_RUNNING);
 puts("OCCAM runtime IN/OUT/block/schedule: PASS");return 0;
}
