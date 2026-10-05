#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "t425_core_phy_ipc.h"
#include "t425_core_phy_rp2040.h"

struct fake_pio_state { int id; };
struct fake_pio_state fake_pio0, fake_pio1;
typedef struct {
    uint32_t rx[8]; unsigned rx_len, rx_pos, ack_count; bool tx_accept;
} link_double_t;
static link_double_t *dof(t425_link_t *l){return (link_double_t *)(uintptr_t)l->rx_buffer;}
bool t425_link_send_frame(t425_link_t*l,uint16_t x){(void)x;return dof(l)->tx_accept;}
bool t425_link_send_ack(t425_link_t*l){++dof(l)->ack_count;return true;}
bool t425_link_rx_available(t425_link_t*l){return dof(l)->rx_pos<dof(l)->rx_len;}
uint32_t t425_link_rx_get(t425_link_t*l){assert(t425_link_rx_available(l));return dof(l)->rx[dof(l)->rx_pos++];}
void t425_link_note_ack_received(t425_link_t*l){(void)l;}
bool t425_link_init(t425_link_t*l,PIO p,uint a,uint b,uint c,uint d,t425_link_speed_t s){(void)l;(void)p;(void)a;(void)b;(void)c;(void)d;(void)s;return true;}
bool t425_link_dma_init(t425_link_t*l,uint32_t*b,uint32_t n){(void)l;(void)b;(void)n;return true;}
void t425_link_dma_deinit(t425_link_t*l){(void)l;} void t425_link_start(t425_link_t*l){(void)l;} void t425_link_stop(t425_link_t*l){(void)l;}
static uint32_t dw(uint8_t b){return (1u<<31)|(1u<<30)|((uint32_t)b<<22);}
static void inject(link_double_t*d,uint8_t b){d->rx[d->rx_len++]=dw(b);}
static void poll(t425_core_phy_ipc_service_t*s){(void)t425_core_phy_ipc_service_poll(s);}
int main(void){
 t425_core_phy_ipc_t ipc; t425_core_phy_ipc_service_t svc; t425_core_phy_t c0; t425_core_phy_rp2040_t ad; t425_core_phy_ops_t o0={0},oa={0};
 t425_link_t links[4]={0}; t425_link_t*lp[4]; t425_core_phy_route_t routes[4]={{0,0},{1,1},{2,2},{3,3}}; link_double_t ds[4]={0};
 for(unsigned i=0;i<4;i++){links[i].pio=(i<2)?pio0:pio1;links[i].sm_rx=(i&1)?3:1;links[i].rx_buffer=(uint32_t*)(uintptr_t)&ds[i];ds[i].tx_accept=true;lp[i]=&links[i];}
 assert(t425_core_phy_ipc_init(&ipc)); t425_core_phy_ipc_make_core0_ops(&ipc,&o0); assert(t425_core_phy_init(&c0,&o0,4));
 assert(t425_core_phy_rp2040_init(&ad,lp,routes)); t425_core_phy_rp2040_make_ops(&ad,&oa); assert(t425_core_phy_ipc_service_init(&svc,&ipc,&oa));
 assert(t425_core_phy_set_rx_arbitration(&c0,true)==0);
 for(unsigned i=0;i<4;i++) assert(t425_core_phy_set_rx_admission_credit(&c0,i,i+1,true)==0);
 inject(&ds[0],0x10); inject(&ds[1],0x20); poll(&svc); poll(&svc);
 uint32_t f=0; assert(t425_core_phy_rx_available(&c0,0)); assert(t425_core_phy_rx(&c0,0,&f)==0&&f==0x10);
 assert(!t425_core_phy_rx_available(&c0,1)); assert(ad.rx_data_pending[1]); assert(ds[0].ack_count==1&&ds[1].ack_count==0);
 poll(&svc); assert(ds[1].ack_count==0);
 assert(t425_core_phy_set_rx_arbitration(&c0,false)==0); poll(&svc);
 assert(t425_core_phy_set_rx_arbitration(&c0,true)==0);
 assert(t425_core_phy_set_rx_admission_credit(&c0,1,2,true)==0); poll(&svc);
 assert(t425_core_phy_rx(&c0,1,&f)==0&&f==0x20); assert(ds[1].ack_count==1); poll(&svc); assert(ds[1].ack_count==1);
 puts("F_PHY-71E global arbitration: PASS");
 return 0;
}
