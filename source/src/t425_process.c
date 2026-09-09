#include "t425_process.h"
#include <string.h>
void t425_process_table_init(t425_runtime_process_t *p){
    memset(p,0,sizeof(t425_runtime_process_t)*T425_MAX_PROCESSES);
    for(uint32_t i=0;i<T425_MAX_PROCESSES;i++){
        p[i].pid=i;p[i].state=PROC_FREE;p[i].sp=T425_WORKSPACE_WORDS;
    }
}
bool t425_process_start(t425_runtime_process_t*p,uint32_t pid,uint32_t pc,uint32_t priority){
    if(pid>=T425_MAX_PROCESSES||p[pid].state!=PROC_FREE)return false;
    p[pid].pc=pc;p[pid].w=0;p[pid].sp=T425_WORKSPACE_WORDS;
    p[pid].priority=priority;p[pid].state=PROC_READY;return true;
}
bool t425_process_stop(t425_runtime_process_t*p,uint32_t pid){
    if(pid>=T425_MAX_PROCESSES||p[pid].state==PROC_FREE)return false;
    p[pid].state=PROC_STOPPED;return true;
}
bool t425_process_run(t425_runtime_process_t*p,uint32_t pid){
    if(pid>=T425_MAX_PROCESSES||p[pid].state!=PROC_STOPPED)return false;
    p[pid].state=PROC_READY;return true;
}
bool t425_process_end(t425_runtime_process_t*p,uint32_t pid){
    if(pid>=T425_MAX_PROCESSES||p[pid].state==PROC_FREE)return false;
    p[pid].state=PROC_TERMINATED;return true;
}
