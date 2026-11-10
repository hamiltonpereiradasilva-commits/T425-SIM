#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>
#include "t425_core.h"
typedef struct { uint8_t q[8]; unsigned n,p; } m_t;
static int tx(void*c,uint32_t e,uint16_t f){(void)c;(void)e;(void)f;return 0;}
static bool av(void*c,uint32_t e){m_t*m=c;return e<4&&m->p<m->n;}
static int get(void*c,uint32_t e,uint32_t*f){m_t*m=c;if(!av(c,e)||!f)return -1;*f=m->q[m->p++];return 0;}
static void put(m_t*m,uint8_t b){m->q[m->n++]=b;}
int main(void){t425_core_t c={0};t425_core_phy_t p={0};m_t m={0};t425_core_phy_ops_t o={tx,av,get,&m,0,0,0,0,0};t425_core_reset(&c);assert(t425_core_phy_init(&p,&o,4));assert(t425_core_attach_phy(&c,&p));c.cpu.IptrReg=T425_INTERNAL_RAM_BASE;t425_memory_write_byte(&c.memory,T425_INTERNAL_RAM_BASE,0x20);t425_memory_write_byte(&c.memory,T425_INTERNAL_RAM_BASE+1,0xF7);c.cpu.A=4; t425_fixture_sync_slaves(&c.cpu);c.cpu.B=0x80000010; t425_fixture_sync_slaves(&c.cpu);c.cpu.C=0x80000200; t425_fixture_sync_slaves(&c.cpu);for(unsigned i=0;i<80;i++)t425_core_step(&c);assert(c.external_in_active[0]&&c.external_in_remaining[0]==4); c.boot_state=T425_BOOT_RUNNING;put(&m,1);put(&m,2);put(&m,3);put(&m,4);uint8_t b;for(unsigned i=0;i<4;i++){assert(t425_core_link_input_consume(&c,0,&b)==0);assert(c.external_in_remaining[0]==3-i);}assert(!c.external_in_active[0]&&c.external_in_remaining[0]==0);assert(t425_core_link_rx_admission(&c,0)==false);puts("F_CORE-IN-01H real multibyte: PASS");return 0;}