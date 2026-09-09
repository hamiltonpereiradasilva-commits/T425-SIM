#include <assert.h>
#include <stdio.h>
#include "t425_occam_text.h"
#include "t425_occam_semantic.h"
#include "t425_occam_compile.h"
int main(void){
 const char*s="CHAN OF INT c0; SEQ { OUT(c0,10); OUT(c0,20); END; }";
 t425_ast_t a;t425_occam_instr_t ir[32];uint16_t n=0;
 assert(t425_occam_parse_text(s,&a));assert(t425_occam_semantic_check(&a));
 assert(t425_occam_compile(&a,ir,32,&n));assert(n==3);
 assert(ir[0].op==OOUT&&ir[0].imm==10);assert(ir[1].imm==20);assert(ir[2].op==OEND);
 puts("OCCAM AST + semantic + compiler: PASS");return 0;
}
