#include <assert.h>
#include <stdio.h>
#include "t425_occam_parser.h"
int main(void){
    const char*s="OUT(0,10) OUT(0,20) END";
    t425_occam_instr_t code[8];size_t n=0;
    assert(t425_occam_parse_demo(s,code,8,&n));
    assert(n==3&&code[0].op==OOUT&&code[1].imm==20&&code[2].op==OEND);
    puts("OCCAM demo parser: PASS");
    return 0;
}
