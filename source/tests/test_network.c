#include <assert.h>
#include <stdio.h>
#include "t425_network.h"
int main(void){uint8_t l,r;t425_net_ring_defaults(0,&l,&r);assert(l==3&&r==1);t425_net_ring_defaults(3,&l,&r);assert(l==2&&r==0);puts("network ring: PASS");}
