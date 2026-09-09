#include "t425_occam_semantic.h"
#include "t425_occam_vm.h"
#include "t425_channels.h"
static bool walk(const t425_ast_t*a,uint16_t i){
 const t425_ast_node_t*n=&a->node[i];
 if(n->kind==AST_IN && n->var>=T425_OCCAM_VARS)return false;
 if((n->kind==AST_IN||n->kind==AST_OUT)&&n->channel>=T425_CHANNELS)return false;
 if(n->kind==AST_TIMER_AFTER&&n->value<0)return false;
 for(uint16_t c=n->first_child;c!=UINT16_MAX;c=a->node[c].next_sibling)
   if(!walk(a,c))return false;
 return true;
}
bool t425_occam_semantic_check(const t425_ast_t*a){
 return a&&a->root<a->count&&walk(a,a->root);
}
