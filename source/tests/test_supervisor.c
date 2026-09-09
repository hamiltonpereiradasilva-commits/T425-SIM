#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_supervisor.h"
int main(void){
 t425_node_config_t c; t425_supervisor_t s; char status[160];
 t425_node_config_init(&c,1);
 t425_node_config_add_link(&c,0,0,0,1,0,0,1);
 t425_node_config_add_link(&c,1,2,2,3,0,2,3);
 assert(t425_supervisor_init(&s,&c));
 assert(s.runtime.state==T425_RT_READY);
 assert(s.scheduler.state==T425_SCHED_READY);
 assert(t425_supervisor_start(&s));
 assert(s.runtime.state==T425_RT_RUNNING);
 for(int i=0;i<10;i++)assert(t425_supervisor_step(&s));
 assert(s.runtime.ticks==10&&s.scheduler.cycles==10);
 assert(t425_supervisor_status(&s,status,sizeof(status)));
 assert(strstr(status,"NODE=1")!=NULL);
 assert(t425_scheduler_fault(&s.scheduler));
 assert(s.scheduler.state==T425_SCHED_FAULT);
 puts("SUPERVISOR + SCHEDULER: PASS");return 0;
}
