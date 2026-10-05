#include <assert.h>
#include "t425_process.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "t425_core.h"
typedef struct { bool reset,rom,busy; uint8_t rx[4][256],tx[4][8]; unsigned len[4],pos[4],txlen[4]; } M;
static int tx(void*c,uint32_t e,uint16_t f){M*m=c;if(e>=4||f>255||m->busy)return -1;m->tx[e][m->txlen[e]++]=(uint8_t)f;return 0;}
static bool av(void*c,uint32_t e){M*m=c;return e<4&&m->pos[e]<m->len[e];}
static int rx(void*c,uint32_t e,uint32_t*f){M*m=c;if(!f||!av(c,e))return -1;*f=m->rx[e][m->pos[e]++];return 0;}
static bool rd(void*c,t425_core_phy_system_inputs_t*i){M*m=c;if(!i)return false;*i=(t425_core_phy_system_inputs_t){0};i->reset=m->reset;i->boot_from_rom=m->rom;return true;}
static bool wr(void*c,const t425_core_phy_system_outputs_t*o){(void)c;return o!=0;}
static void put(M*m,unsigned l,uint8_t b){m->rx[l][m->len[l]++]=b;}
static void word(M*m,unsigned l,uint32_t v){put(m,l,v);put(m,l,v>>8);put(m,l,v>>16);put(m,l,v>>24);}
static void initc(t425_core_t*c,t425_core_phy_t*p,M*m){t425_core_phy_ops_t o={tx,av,rx,m,rd,wr,0,0};memset(c,0,sizeof*c);memset(p,0,sizeof*p);memset(m,0,sizeof*m);t425_core_reset(c);assert(t425_core_phy_init(p,&o,4));assert(t425_core_attach_phy(c,p));}
static void start(t425_core_t*c,M*m,bool rom){m->reset=true;assert(t425_core_step(c)==0);m->reset=false;m->rom=rom;assert(t425_core_step(c)==0);assert(t425_core_step(c)==0);}
static void test_rom(void)
{
    t425_core_t c;
    t425_core_phy_t p;
    M m;
    uint8_t b;

    initc(&c, &p, &m);
    assert(t425_memory_write_byte(&c.memory, T425_MEM_START, 0xa5) == 0);

    c.process_contexts[0].valid = 1u;
    c.process_contexts[0].wptr = 0x11110000u;
    c.process_contexts[0].Wdesc = 0x22220001u;
    c.process_contexts[0].Iptr = 0x33330000u;
    c.interrupt_contexts[0].valid = 1u;
    c.interrupt_contexts[0].WdescIntSave = 0x44440001u;
    c.interrupt_contexts[0].IptrIntSave = 0x55550000u;

    start(&c, &m, true);

    assert(c.boot_state == T425_BOOT_RUNNING);
    assert(c.cpu.IptrReg == T425_ROM_ENTRY);
    assert(c.cpu.WptrReg == T425_MEM_START);
    assert(c.cpu.WdescReg == (T425_MEM_START | T425_PRIORITY_LOW));
    assert(c.scheduler.running == 1u);
    assert(c.scheduler.current_wdesc == c.cpu.WdescReg);
    assert(t425_wdesc_priority(c.cpu.WdescReg) == T425_PRIORITY_LOW);
    assert(t425_wdesc_wptr(c.cpu.WdescReg) == T425_MEM_START);

    assert(c.process_contexts[0].valid == 0u);
    assert(c.process_contexts[0].wptr == 0u);
    assert(c.process_contexts[0].Wdesc == 0u);
    assert(c.process_contexts[0].Iptr == 0u);
    assert(c.interrupt_contexts[0].valid == 0u);
    assert(c.interrupt_contexts[0].WdescIntSave == 0u);
    assert(c.interrupt_contexts[0].IptrIntSave == 0u);

    m.rom = false;
    assert(t425_memory_read_byte(&c.memory, T425_MEM_START, &b) == 0);
    assert(b == 0xa5);
}
static void test_links(void){for(unsigned l=0;l<4;l++){t425_core_t c;t425_core_phy_t p;M m;initc(&c,&p,&m);put(&m,l,0);start(&c,&m,false);assert(t425_core_step(&c)==0);assert(c.bootstrap_link==l&&c.boot_state==T425_BOOT_POKE_ADDRESS);}}
static void test_poke(void){t425_core_t c;t425_core_phy_t p;M m;uint32_t v;initc(&c,&p,&m);put(&m,2,0);word(&m,2,T425_MEM_START+0x20);word(&m,2,0x44332211);start(&c,&m,false);assert(t425_core_step(&c)==0);put(&m,1,0xff);for(unsigned i=0;i<8;i++)assert(t425_core_step(&c)==0);assert(m.pos[1]==0&&c.boot_state==T425_BOOT_WAIT_CONTROL);assert(t425_memory_read_word(&c.memory,T425_MEM_START+0x20,&v)==0&&v==0x44332211);}
static void test_peek(void){t425_core_t c;t425_core_phy_t p;M m;initc(&c,&p,&m);put(&m,3,1);word(&m,3,T425_MEM_START);assert(t425_memory_write_word(&c.memory,T425_MEM_START,0x04030201)==0);start(&c,&m,false);for(unsigned i=0;i<5;i++)assert(t425_core_step(&c)==0);m.busy=true;assert(t425_core_step(&c)==0);assert(c.bootstrap_byte_index==0);m.busy=false;for(unsigned i=0;i<4;i++)assert(t425_core_step(&c)==0);assert(c.boot_state==T425_BOOT_WAIT_CONTROL&&m.txlen[3]==4&&m.tx[3][0]==1&&m.tx[3][3]==4);}
static void test_load(void){t425_core_t c;t425_core_phy_t p;M m;uint8_t b;initc(&c,&p,&m);put(&m,1,3);put(&m,1,0xaa);put(&m,1,0xbb);put(&m,1,0xcc);start(&c,&m,false);assert(t425_core_step(&c)==0);put(&m,0,0xee);for(unsigned i=0;i<3;i++)assert(t425_core_step(&c)==0);assert(c.boot_state==T425_BOOT_RUNNING&&m.pos[0]==0&&
           c.cpu.IptrReg==T425_MEM_START&&
           c.cpu.WptrReg==T425_MEM_START+4);
    assert(c.cpu.WdescReg == ((T425_MEM_START + 4u) | T425_PRIORITY_LOW));
    assert(c.scheduler.running == 1u);
    assert(c.scheduler.current_wdesc == c.cpu.WdescReg);
    assert(t425_wdesc_priority(c.cpu.WdescReg) == T425_PRIORITY_LOW);
    assert(t425_wdesc_wptr(c.cpu.WdescReg) == T425_MEM_START + 4u);assert(t425_memory_read_byte(&c.memory,T425_MEM_START,&b)==0&&b==0xaa);assert(t425_memory_read_byte(&c.memory,T425_MEM_START+2,&b)==0&&b==0xcc);}
int main(void){test_rom();test_links();test_poke();test_peek();test_load();puts("F_CORE-SYS-01 bootstrap: PASS");return 0;}