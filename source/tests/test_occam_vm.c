#include <assert.h>
#include <stdio.h>
#include "t425_node_sim.h"
int main(void){
    static const t425_occam_instr_t tx[]={{OOUT,0,0,10},{OOUT,0,0,20},{OEND,0,0,0}};
    static const t425_occam_instr_t rx[]={{OIN,0,0,0},{OIN,0,1,0},{OEND,0,0,0}};
    t425_occam_program_t a={"sender",tx,3},b={"receiver",rx,3};
    t425_node_pair_t n;t425_node_pair_init(&n,&a,&b);
    int rc=t425_node_pair_run(&n,100);
    assert(rc>0);
    assert(n.vm[0].state==OVM_DONE&&n.vm[1].state==OVM_DONE);
    assert(n.vm[1].vars[0]==10&&n.vm[1].vars[1]==20);
    puts("OCCAM VM PAR sender/receiver: PASS");
    return 0;
}
