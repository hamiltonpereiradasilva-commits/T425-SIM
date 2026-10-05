#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "t425_core.h"
#include "t425_channel.h"

typedef struct {
    bool reset;
    uint8_t rx[4][32];
    unsigned len[4], pos[4];
    uint32_t generation[4];
    bool enabled[4];
} mock_t;

static int tx(void *c, uint32_t e, uint16_t f){(void)c;(void)e;(void)f;return 0;}
static bool av(void *c,uint32_t e){mock_t*m=c;return e<4u&&m->pos[e]<m->len[e];}
static int get(void*c,uint32_t e,uint32_t*f){mock_t*m=c;if(!f||!av(c,e))return -1;*f=m->rx[e][m->pos[e]++];return 0;}
static bool rd(void*c,t425_core_phy_system_inputs_t*i){mock_t*m=c;*i=(t425_core_phy_system_inputs_t){0};i->reset=m->reset;return true;}
static bool wr(void*c,const t425_core_phy_system_outputs_t*o){(void)c;return o!=0;}
static int credit(void*c,uint32_t e,uint32_t g,bool en){mock_t*m=c;if(e>=4u)return -1;m->generation[e]=g;m->enabled[e]=en;return 0;}
static void put(mock_t*m,unsigned e,uint8_t b){m->rx[e][m->len[e]++]=b;}

int main(void)
{
    t425_core_t core; t425_core_phy_t phy; mock_t m={0};
    t425_core_phy_ops_t ops={tx,av,get,&m,rd,wr,0,credit};
    memset(&core,0,sizeof core); memset(&phy,0,sizeof phy);
    t425_core_reset(&core);
    assert(t425_core_phy_init(&phy,&ops,4u));
    assert(t425_core_attach_phy(&core,&phy));
    assert(t425_memory_write_word(&core.memory,0x80000010u,T425_CHANNEL_EMPTY)==0);

    m.reset=true; assert(t425_core_step(&core)==0);
    m.reset=false; assert(t425_core_step(&core)==0);
    assert(t425_core_step(&core)==0);
    for(unsigned i=0;i<4;i++) assert(m.enabled[i]);

    put(&m,0,3); put(&m,0,0x11); put(&m,0,0x22); put(&m,0,0x33); put(&m,0,0xee);
    assert(t425_core_step(&core)==0);
    assert(core.bootstrap_link==0u && m.enabled[0]);
    assert(!m.enabled[1]&&!m.enabled[2]&&!m.enabled[3]);
    assert(t425_core_step(&core)==0);
    assert(t425_core_step(&core)==0);
    assert(t425_core_step(&core)==0);
    assert(core.boot_state==T425_BOOT_RUNNING);
    assert(!m.enabled[0]);
    assert(m.pos[0]==4u);

    assert(t425_memory_write_word(&core.memory,0x80000010u,0x1234u)==0);
    assert(t425_core_step(&core)==0);
    assert(m.enabled[0]);
    {
        uint8_t byte=0u;
        assert(t425_core_link_input_consume(&core,0u,&byte)==0);
        assert(byte==0xee);
    }
    assert(t425_channel_take(&core.memory,0x80000010u,&(uint32_t){0})==0);
    assert(t425_core_step(&core)==0);
    assert(!m.enabled[0]);
    puts("F_PHY-71C credit: PASS");
    return 0;
}
