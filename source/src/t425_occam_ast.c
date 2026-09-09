#include "t425_occam_ast.h"
#include <string.h>
void t425_ast_init(t425_ast_t*a){memset(a,0,sizeof(*a));a->root=UINT16_MAX;
 for(int i=0;i<T425_AST_MAX_NODES;i++){a->node[i].first_child=UINT16_MAX;a->node[i].next_sibling=UINT16_MAX;}}
int t425_ast_add(t425_ast_t*a,t425_ast_node_t n){
 if(!a||a->count>=T425_AST_MAX_NODES)return -1;
 n.first_child=UINT16_MAX;n.next_sibling=UINT16_MAX;
 a->node[a->count]=n;return (int)a->count++;
}
bool t425_ast_add_child(t425_ast_t*a,uint16_t parent,uint16_t child){
 if(!a||parent>=a->count||child>=a->count)return false;
 if(a->node[parent].first_child==UINT16_MAX){a->node[parent].first_child=child;return true;}
 uint16_t x=a->node[parent].first_child;
 while(a->node[x].next_sibling!=UINT16_MAX)x=a->node[x].next_sibling;
 a->node[x].next_sibling=child;return true;
}
