#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "t425_core.h"
#include "t425_channel.h"
typedef struct { uint8_t q[8]; unsigned n,p; } m_t;
static int tx(void*c,uint32_t e,uint16_t f){(void)c;(void)e;(void)f;return 0;}
static bool av(void*c,uint32_t e){m_t*m=c;return e<4&&m->p<m->n;}
static int get(void*c,uint32_t e,uint32_t*f){m_t*m=c;if(!av(c,e)||!f)return -1;*f=m->q[m->p++];return 0;}
static void put(m_t*m,uint8_t b){m->q[m->n++]=b;}
int main(void){
 t425_core_t c={0}; t425_core_phy_t p={0}; m_t m={0}; t425_core_phy_ops_t o={tx,av,get,&m,0,0,0,0,0};
 t425_core_reset(&c); assert(t425_core_phy_init(&p,&o,4)); assert(t425_core_attach_phy(&c,&p)); c.boot_state=T425_BOOT_RUNNING;
 assert(t425_memory_write_word(&c.memory,0x80000010u,T425_CHANNEL_EMPTY)==0);
 assert(t425_core_external_in_begin(&c,0,2)==0);
 assert(t425_core_link_rx_admission(&c,0));
 put(&m,0x11); put(&m,0x22); put(&m,0x33);
 uint8_t b=0; assert(t425_core_link_input_consume(&c,0,&b)==0&&b==0x11);
 assert(t425_core_external_in_remaining(&c,0)==1); assert(t425_core_link_rx_admission(&c,0));
 assert(t425_core_link_input_consume(&c,0,&b)==0&&b==0x22);
 assert(t425_core_external_in_remaining(&c,0)==0); assert(!t425_core_link_rx_admission(&c,0));
 assert(t425_core_link_input_consume(&c,0,&b)==0&&b==0x33);
 puts("F_PHY-71E IN lifetime: PASS"); return 0;
}
