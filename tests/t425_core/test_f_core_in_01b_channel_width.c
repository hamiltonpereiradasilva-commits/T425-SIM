#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "t425_mir_fields.h"
static void set_channel(t425_mir_t *mir, uint16_t value){static const unsigned pos[]={74u,11u,10u,9u,7u,6u,5u,4u,3u,2u,1u};for(unsigned i=0;i<11u;++i)mir->bit[pos[i]]=(uint8_t)((value>>(10u-i))&1u);}
int main(void){static const uint16_t v[]={1u,2u,4u,8u,0x10u,0x20u,0x40u,0x80u,0x100u,0x200u,0x400u};t425_mir_t m={{0}};for(unsigned i=0;i<sizeof(v)/sizeof(v[0]);++i){m=(t425_mir_t){{0}};set_channel(&m,v[i]);assert(t425_mir_channel(&m)==v[i]);}puts("F_CORE-IN-01B channel width: PASS");return 0;}
