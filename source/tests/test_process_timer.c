#include <assert.h>
#include <stdio.h>
#include "t425_process.h"
#include "t425_timer.h"

int main(void){
    t425_runtime_process_t p[T425_MAX_PROCESSES];
    t425_process_table_init(p);
    assert(p[0].state==PROC_FREE);
    assert(t425_process_start(p,0,0x80000070,1));
    assert(p[0].state==PROC_READY && p[0].priority==1);
    assert(t425_process_stop(p,0));
    assert(t425_process_run(p,0));
    assert(t425_process_end(p,0));
    assert(p[0].state==PROC_TERMINATED);

    t425_timer_t t;
    t425_timer_init(&t);
    assert(t425_timer_tin(&t,0,100));
    assert(!t425_timer_ready(&t,0));
    t425_timer_tick(&t,99);
    assert(!t425_timer_ready(&t,0));
    t425_timer_tick(&t,1);
    assert(t425_timer_ready(&t,0));
    t425_timer_enbt(&t,0);
    assert(!t.guard[0].enabled);
    puts("process descriptor + timer smoke: PASS");
    return 0;
}
