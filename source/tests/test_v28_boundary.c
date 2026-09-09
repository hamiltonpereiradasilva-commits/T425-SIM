#include <assert.h>
#include <stdio.h>
#include "t425_link.h"
int main(void){
    t425_link_stack_t a,b; t425_frame_t f; uint16_t n;
    const uint8_t p[]={0,1,2,3,4,5,6,7};
    t425_link_stack_init(&a); t425_link_stack_init(&b);
    assert(t425_link_stack_prepare(&a,0,1,p,sizeof(p),&n));
    assert(t425_link_stack_receive(&b,a.tx_buf,n,&f));
    assert(f.src==0&&f.dst==1&&f.seq==0&&f.len==sizeof(p));
    puts("V2.8 LINK-STACK BOUNDARY: PASS");
    return 0;
}
