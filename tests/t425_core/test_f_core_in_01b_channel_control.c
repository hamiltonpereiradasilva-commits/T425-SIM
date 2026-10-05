#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "t425_core.h"
#include "t425_mir_fields.h"
static void set_channel(t425_mir_t *mir,uint16_t value){static const unsigned pos[]={74u,11u,10u,9u,7u,6u,5u,4u,3u,2u,1u};for(unsigned i=0;i<11u;++i)mir->bit[pos[i]]=(uint8_t)((value>>(10u-i))&1u);}
static void run(t425_core_t*c,uint16_t v){c->microsequencer.mir=(t425_mir_t){{0}};set_channel(&c->microsequencer.mir,v);t425_core_microcycle(c);}
int main(void){t425_core_t c={0};t425_core_reset(&c);c.t425_data_reg=0xA5u;c.t425_channel_select=0x80000010u;run(&c,0x400u);assert(c.t425_channel_data_reg==0xA5u);run(&c,1u);assert(c.t425_data_mem_addr==0x80000010u);run(&c,0x20u);assert(c.t425_alt_enabled==1u);run(&c,0x10u);assert(c.t425_alt_enabled==0u);run(&c,4u);assert(c.t425_channel_status==1u);run(&c,8u);assert(c.t425_channel_status==0u);c.t425_channel_ready_not_run=1u;run(&c,0x40u);assert(c.t425_channel_ready_not_run==0u);c.t425_channel_run_req=1u;run(&c,2u);assert(c.t425_channel_run_req==0u);puts("F_CORE-IN-01B channel controls: PASS");return 0;}
