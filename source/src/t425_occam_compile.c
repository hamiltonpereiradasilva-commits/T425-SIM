#include "t425_occam_compile.h"
static bool emit(t425_occam_instr_t*out,uint16_t cap,uint16_t*n,t425_occam_instr_t i){
    if(*n>=cap) return false;
    out[(*n)++]=i;
    return true;
}
static bool walk(const t425_ast_t*a,uint16_t idx,t425_occam_instr_t*out,
                 uint16_t cap,uint16_t*n){
    const t425_ast_node_t*x=&a->node[idx];
    switch(x->kind){
        case AST_SEQ:
        case AST_PAR:
            for(uint16_t c=x->first_child;c!=UINT16_MAX;c=a->node[c].next_sibling) if(!walk(a,c,out,cap,n)) return false;
            return true;
        case AST_IN:
            return emit(out,cap,n,(t425_occam_instr_t){OIN,x->channel,x->var,0});
        case AST_OUT:
            return emit(out,cap,n,(t425_occam_instr_t){OOUT,x->channel,0,(uint32_t)x->value});
        case AST_TIMER_AFTER:
            return emit(out,cap,n,(t425_occam_instr_t){OTIMER_AFTER,(uint16_t)x->value,0,0});
        case AST_END:
            return emit(out,cap,n,(t425_occam_instr_t){OEND,0,0,0});
        default:
            return true;
    }
}
bool t425_occam_compile(const t425_ast_t*a,t425_occam_instr_t*out,
                        uint16_t cap,uint16_t*n){
    if(!a||!out||!n||a->root==UINT16_MAX||a->root>=a->count) return false;
    *n=0;
    return walk(a,a->root,out,cap,n);
}
