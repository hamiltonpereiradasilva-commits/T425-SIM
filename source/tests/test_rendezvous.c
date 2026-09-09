#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_rendezvous.h"
int main(void){
 t425_rendezvous_t r;t425_occam_message_t m;uint8_t op,ip;uint8_t x[]={42,43};
 t425_rdv_init(&r);
 assert(t425_rdv_post_out(&r,0,x,2));assert(r.state==T425_RDV_WAIT_OUT);
 assert(t425_rdv_post_in(&r,1));assert(r.state==T425_RDV_TRANSFER);
 assert(t425_rdv_transfer(&r,&m,&op,&ip));
 assert(op==0&&ip==1&&m.len==2&&m.value[0]==42&&m.value[1]==43);
 assert(r.state==T425_RDV_DONE);
 t425_rdv_init(&r);assert(t425_rdv_post_in(&r,2));assert(t425_rdv_post_out(&r,3,x,2));
 assert(t425_rdv_transfer(&r,&m,&op,&ip));assert(op==3&&ip==2);
 puts("OCCAM rendezvous OUT/IN ordering: PASS");return 0;
}
