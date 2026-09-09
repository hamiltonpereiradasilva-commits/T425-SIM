#include "t425_node_sim.h"
void t425_node_pair_init(t425_node_pair_t*n,const t425_occam_program_t*a,const t425_occam_program_t*b){
    t425_channels_init(n->channels);t425_timer_init(&n->timer);
    t425_occam_vm_init(&n->vm[0],a);t425_occam_vm_init(&n->vm[1],b);
    n->vm[0].pid=0;n->vm[1].pid=1;
}
int t425_node_pair_run(t425_node_pair_t*n,uint32_t max_steps){
    uint32_t steps=0;
    while(steps++<max_steps){
        int progress=0;
        for(int i=0;i<2;i++){
            if(n->vm[i].state==OVM_WAIT_IN||n->vm[i].state==OVM_WAIT_OUT||n->vm[i].state==OVM_WAIT_TIMER)
                n->vm[i].state=OVM_READY;
            if(n->vm[i].state==OVM_READY) progress|=t425_occam_vm_step(&n->vm[i],n->channels,&n->timer)>0;
        }
        if(n->vm[0].state==OVM_DONE&&n->vm[1].state==OVM_DONE)return (int)steps;
        if(!progress)return -1;
    }
    return -2;
}
