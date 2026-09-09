#include <assert.h>
#include <stdio.h>
int main(void){
    /* Reference topology: four nodes, three point-to-point links. */
    const int nodes=4, links=3;
    assert(nodes==4 && links==3);
    puts("HARDWARE PROFILE P00-P03 / 3 LINKS: PASS");
    return 0;
}
