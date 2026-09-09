#include "t425_occam_vm.h"
static void fail(t425_occam_vm_t*v){v->state=OVM_ERROR;}
void t425_occam_vm_init(t425_occam_vm_t*v,const t425_occam_program_t*p){
    v->pid=0;v->pc=0;v->state=OVM_READY;v->wait_channel=0;v->wait_timer=0;v->program=p;
    for(int i=0;i<T425_OCCAM_VARS;i++)v->vars[i]=0;
}
int t425_occam_vm_step(t425_occam_vm_t*v,t425_channel_t*c,t425_timer_t*t){
    if(!v||!v->program||v->state==OVM_DONE||v->state==OVM_ERROR)return 0;
    if(v->pc>=v->program->length){v->state=OVM_DONE;return 0;}
    const t425_occam_instr_t*i=&v->program->code[v->pc];
    switch(i->op){
        case OOUT:
            if(i->arg0>=T425_CHANNELS){fail(v);return -1;}
            if(!t425_chan_out(&c[i->arg0],v->pid,i->imm)){
                v->state=OVM_WAIT_OUT;v->wait_channel=i->arg0;return 0;
            }
            v->pc++;return 1;
        case OIN: {
            if(i->arg0>=T425_CHANNELS||i->arg1>=T425_OCCAM_VARS){fail(v);return -1;}
            uint32_t x=0;
            if(!t425_chan_in(&c[i->arg0],v->pid,&x)){
                v->state=OVM_WAIT_IN;v->wait_channel=i->arg0;return 0;
            }
            v->vars[i->arg1]=x;v->pc++;return 1;
        }
        case OTIMER_AFTER:
            if(i->arg0>=T425_MAX_ALTS){fail(v);return -1;}
            if(!t425_timer_ready(t,i->arg0)){
                v->state=OVM_WAIT_TIMER;v->wait_timer=i->arg0;return 0;
            }
            v->pc++;return 1;
        case OSEQ: v->pc++;return 1;
        case OALT: v->pc++;return 1; /* selection handled by ALT layer in next revision */
        case OEND: v->state=OVM_DONE;return 0;
        default: fail(v);return -1;
    }
}
