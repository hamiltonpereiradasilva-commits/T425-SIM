#include "t425_parallel_vm.h"
static bool all_done(const t425_parallel_vm_t*v){
 for(uint16_t i=0;i<v->image.process_count;i++)if(v->vm[i].state!=OVM_DONE)return false;
 return true;
}
bool t425_parallel_vm_init(t425_parallel_vm_t*v,const t425_ast_t*a){
 if(!v||!t425_occam_compile_parallel(a,&v->image))return false;
 t425_channels_init(v->channels);t425_timer_init(&v->timer);v->rr=0;
 for(uint16_t i=0;i<v->image.process_count;i++){
  t425_process_image_t*p=&v->image.process[i];
  v->program[i].name=0;v->program[i].code=&v->image.ir[p->ir_offset];v->program[i].length=p->ir_count;
  t425_occam_vm_init(&v->vm[i],&v->program[i]);v->vm[i].pid=p->pid;
 }
 return true;
}
static bool rendezvous(t425_parallel_vm_t*v){
 bool hit=false;
 for(uint16_t a=0;a<v->image.process_count;a++){
  t425_occam_vm_t*pa=&v->vm[a];
  if(pa->state==OVM_DONE||pa->pc>=pa->program->length)continue;
  const t425_occam_instr_t*ia=&pa->program->code[pa->pc];
  if(ia->op!=OIN)continue;
  for(uint16_t b=0;b<v->image.process_count;b++){
   t425_occam_vm_t*pb=&v->vm[b];
   if(a==b||pb->state==OVM_DONE||pb->pc>=pb->program->length)continue;
   const t425_occam_instr_t*ib=&pb->program->code[pb->pc];
   if(ib->op==OOUT&&ib->arg0==ia->arg0){
    if(ia->arg1<T425_OCCAM_VARS){pa->vars[ia->arg1]=ib->imm;pa->pc++;pa->state=OVM_READY;}
    pb->pc++;pb->state=OVM_READY;hit=true;break;
   }
  }
 }
 return hit;
}
int t425_parallel_vm_run(t425_parallel_vm_t*v,uint32_t max_cycles){
 for(uint32_t cycle=0;cycle<max_cycles;cycle++){
  if(all_done(v))return (int)cycle;
  bool progress=rendezvous(v);
  for(uint16_t k=0;k<v->image.process_count;k++){
   uint16_t i=(uint16_t)((v->rr+k)%v->image.process_count);
   if(v->vm[i].state==OVM_DONE)continue;
   if(v->vm[i].state==OVM_WAIT_TIMER)v->vm[i].state=OVM_READY;
   if(v->vm[i].state==OVM_READY&&v->vm[i].pc<v->vm[i].program->length){
    const t425_occam_instr_t*x=&v->vm[i].program->code[v->vm[i].pc];
    if(x->op!=OIN&&x->op!=OOUT){
     int rc=t425_occam_vm_step(&v->vm[i],v->channels,&v->timer);
     if(rc<0)return -1;
     if(rc>0)progress=true;
    }else if(x->op==OOUT){
     /* If no receiver is ready, leave process blocked. */
     v->vm[i].state=OVM_WAIT_OUT;
    }else{
     v->vm[i].state=OVM_WAIT_IN;
    }
   }
  }
  v->rr=(uint16_t)((v->rr+1)%v->image.process_count);
  t425_timer_tick(&v->timer,1);
  if(!progress && !rendezvous(v) && !all_done(v)){
   /* A blocked communication may become possible on the next scheduling round. */
  }
 }
 return -3;
}
