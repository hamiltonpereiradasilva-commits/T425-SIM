#include "t425_occam_parallel.h"
#include "t425_occam_semantic.h"
static bool emit(t425_parallel_image_t*i,t425_occam_instr_t x){
 if(i->ir_count>=T425_MAX_PROCESSES*T425_MAX_IR_PER_PROCESS)return false;
 i->ir[i->ir_count++]=x;return true;
}
static bool linear(const t425_ast_t*a,uint16_t idx,t425_parallel_image_t*i){
 const t425_ast_node_t*n=&a->node[idx];
 if(n->kind==AST_SEQ){
   for(uint16_t c=n->first_child;c!=UINT16_MAX;c=a->node[c].next_sibling)
     if(!linear(a,c,i))return false;
   return true;
 }
 if(n->kind==AST_OUT)return emit(i,(t425_occam_instr_t){OOUT,n->channel,0,(uint32_t)n->value});
 if(n->kind==AST_IN)return emit(i,(t425_occam_instr_t){OIN,n->channel,n->var,0});
 if(n->kind==AST_TIMER_AFTER)return emit(i,(t425_occam_instr_t){OTIMER_AFTER,(uint16_t)n->value,0,0});
 if(n->kind==AST_END)return emit(i,(t425_occam_instr_t){OEND,0,0,0});
 return false;
}
static bool make_process(const t425_ast_t*a,uint16_t root,t425_parallel_image_t*i){
 if(i->process_count>=T425_MAX_PROCESSES)return false;
 t425_process_image_t*p=&i->process[i->process_count];
 p->pid=i->process_count;p->ir_offset=i->ir_count;
 if(!linear(a,root,i))return false;
 p->ir_count=i->ir_count-p->ir_offset;
 i->process_count++;return true;
}
bool t425_occam_compile_parallel(const t425_ast_t*a,t425_parallel_image_t*i){
 if(!a||!i||a->root>=a->count||!t425_occam_semantic_check(a))return false;
 i->process_count=0;i->ir_count=0;
 const t425_ast_node_t*r=&a->node[a->root];
 if(r->kind!=AST_PAR)return make_process(a,a->root,i);
 for(uint16_t c=r->first_child;c!=UINT16_MAX;c=a->node[c].next_sibling)
   if(!make_process(a,c,i))return false;
 return i->process_count>0;
}
