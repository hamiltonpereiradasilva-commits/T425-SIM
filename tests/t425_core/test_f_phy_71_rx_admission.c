#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "t425_core.h"
#include "t425_channel.h"

typedef struct { bool admission[4]; } mock_t;
static int tx(void *ctx, uint32_t e, uint16_t f){(void)ctx;(void)e;(void)f;return 0;}
static bool av(void *ctx, uint32_t e){(void)ctx;(void)e;return false;}
static int rx(void *ctx, uint32_t e, uint32_t *f){(void)ctx;(void)e;(void)f;return -1;}
static int set_admission(void *ctx, uint32_t e, bool enabled){mock_t*m=ctx;if(e>=4u)return -1;m->admission[e]=enabled;return 0;}

static void test_input_words(void)
{
    t425_core_t core; uint32_t taken = 0u;
    memset(&core,0,sizeof core); t425_core_reset(&core);
    for(uint32_t i=0u;i<4u;++i){
        assert(t425_memory_write_word(&core.memory,0x80000010u+i*4u,T425_CHANNEL_EMPTY)==0);
        assert(!t425_core_link_rx_admission(&core,i));
        assert(t425_memory_write_word(&core.memory,0x80000010u+i*4u,0x100u+i)==0);
        assert(t425_core_link_rx_admission(&core,i));
    }
    assert(t425_memory_write_word(&core.memory,0x80000000u,0x1234u)==0);
    assert(t425_memory_write_word(&core.memory,0x80000010u,T425_CHANNEL_EMPTY)==0);
    assert(!t425_core_link_rx_admission(&core,0u));
    assert(t425_memory_write_word(&core.memory,0x80000010u,0x220u)==0);
    assert(t425_channel_take(&core.memory,0x80000010u,&taken)==0);
    assert(taken==0x220u);
    assert(!t425_core_link_rx_admission(&core,0u));
}

static void test_portable_publication(void)
{
    mock_t mock={{false,false,false,false}};
    t425_core_phy_ops_t ops={tx,av,rx,&mock,0,0,set_admission,0};
    t425_core_phy_t phy;
    assert(t425_core_phy_init(&phy,&ops,4u));
    for(uint32_t i=0u;i<4u;++i)
        assert(t425_core_phy_set_rx_admission(&phy,i,true)==0&&mock.admission[i]);
    assert(t425_core_phy_set_rx_admission(&phy,4u,true)!=0);
}

int main(void){test_input_words();test_portable_publication();puts("F_PHY-71 RX admission: PASS");return 0;}
