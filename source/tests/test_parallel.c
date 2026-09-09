#include <assert.h>
#include <stdio.h>
#include "t425_occam_text.h"
#include "t425_parallel_vm.h"
int main(void){
 const char*s="PAR { SEQ { OUT(c0,10); OUT(c0,20); END; } SEQ { IN(c0,v0); IN(c0,v1); END; } }";
 t425_ast_t a; assert(t425_occam_parse_text(s,&a));
 t425_parallel_vm_t v; assert(t425_parallel_vm_init(&v,&a));
 assert(v.image.process_count==2);
 assert(v.image.process[0].ir_count==3&&v.image.process[1].ir_count==3);
 assert(t425_parallel_vm_run(&v,100)>0);
 assert(v.vm[0].state==OVM_DONE&&v.vm[1].state==OVM_DONE);
 assert(v.vm[1].vars[0]==10&&v.vm[1].vars[1]==20);
 puts("OCCAM PAR -> process images -> scheduler: PASS");return 0;
}
