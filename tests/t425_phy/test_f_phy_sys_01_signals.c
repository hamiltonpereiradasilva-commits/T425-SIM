#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "t425_core_phy.h"
typedef struct { t425_core_phy_system_inputs_t inputs; t425_core_phy_system_outputs_t outputs; unsigned reads; unsigned writes; } mock_t;
static int tx(void *c,uint32_t e,uint16_t f){(void)c;(void)e;(void)f;return 0;}
static bool av(void *c,uint32_t e){(void)c;(void)e;return false;}
static int get(void *c,uint32_t e,uint32_t *f){(void)c;(void)e;(void)f;return -1;}
static bool rd(void *c,t425_core_phy_system_inputs_t *o){mock_t*m=c;if(!o)return false;*o=m->inputs;++m->reads;return true;}
static bool wr(void *c,const t425_core_phy_system_outputs_t *o){mock_t*m=c;if(!o)return false;m->outputs=*o;++m->writes;return true;}
int main(void){
 mock_t m={0}; t425_core_phy_ops_t ops={tx,av,get,&m,rd,wr}; t425_core_phy_t p; t425_core_phy_system_inputs_t in; t425_core_phy_system_outputs_t out={true,false,true};
 assert(t425_core_phy_init(&p,&ops,4u)); assert(t425_core_phy_read_system_inputs(&p,&in)); assert(m.reads==1u);
 m.inputs.reset=true;m.inputs.analyse=true;m.inputs.error_in=true;m.inputs.boot_from_rom=true;m.inputs.event_req=true;m.inputs.disable_int_ram=true;
 m.inputs.proc_speed_select[0]=true;m.inputs.proc_speed_select[2]=true;m.inputs.link_special=true;m.inputs.link123_special=true;
 assert(t425_core_phy_read_system_inputs(&p,&in)); assert(memcmp(&in,&m.inputs,sizeof in)==0);
 assert(t425_core_phy_write_system_outputs(&p,&out)); assert(m.writes==1u); assert(m.outputs.error&&!m.outputs.event_ack&&m.outputs.event_waiting);
 m.inputs.analyse=false; assert(t425_core_phy_read_system_inputs(&p,&in)); assert(in.reset&&!in.analyse&&in.error_in&&in.proc_speed_select[0]&&!in.proc_speed_select[1]&&in.proc_speed_select[2]&&in.link_special&&!in.link0_special&&in.link123_special);
 assert(!t425_core_phy_read_system_inputs(&p,0)); assert(!t425_core_phy_write_system_outputs(&p,0)); assert(!t425_core_phy_read_system_inputs(0,&in));
 { t425_core_phy_t q; t425_core_phy_ops_t legacy={tx,av,get,&m,0,0}; assert(t425_core_phy_init(&q,&legacy,4u)); assert(!t425_core_phy_read_system_inputs(&q,&in)); assert(!t425_core_phy_write_system_outputs(&q,&out)); }
 return 0;
}
