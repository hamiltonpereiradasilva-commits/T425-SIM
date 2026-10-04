#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "t425_link.h"
#include "hardware/dma.h"

static test_pio_hw_t hw;
PIO test_pio = &hw;
int test_dma_fail_claim_number = -1;
int test_dma_claim_count;
int test_dma_abort_count;
int test_dma_unclaim_count;
static int dma_start_count;

int dma_claim_unused_channel(bool required) { (void)required; return test_dma_claim_count++; }
void dma_channel_unclaim(uint c) { (void)c; test_dma_unclaim_count++; }
void dma_channel_abort(uint c) { (void)c; test_dma_abort_count++; }
dma_channel_config dma_channel_get_default_config(uint c) { (void)c; return (dma_channel_config){0}; }
void channel_config_set_transfer_data_size(dma_channel_config *c,uint s){c->size=s;}
void channel_config_set_read_increment(dma_channel_config *c,bool v){c->read_inc=v;}
void channel_config_set_write_increment(dma_channel_config *c,bool v){c->write_inc=v;}
void channel_config_set_dreq(dma_channel_config *c,uint v){c->dreq=v;}
void dma_channel_configure(uint c,const dma_channel_config *cfg,volatile void *w,const volatile void *r,uint32_t n,bool s){(void)c;(void)cfg;(void)w;(void)r;(void)n;(void)s;}
void dma_channel_set_read_addr(uint c,const volatile void *r,bool t){(void)c;(void)r;(void)t;}
void dma_channel_set_trans_count(uint c,uint32_t n,bool t){(void)c;(void)n;(void)t; dma_start_count++;}

uint pio_add_program(PIO p,const pio_program *program){(void)p;(void)program;return 0u;}
void pio_gpio_init(PIO p,uint pin){(void)p;(void)pin;}
void pio_sm_set_consecutive_pindirs(PIO p,uint sm,uint pin,uint n,bool out){(void)p;(void)sm;(void)pin;(void)n;(void)out;}
void pio_sm_init(PIO p,uint sm,uint off,const pio_sm_config *c){(void)p;(void)sm;(void)off;(void)c;}
void pio_enable_sm_mask_in_sync(PIO p,uint m){(void)p;(void)m;}
void pio_set_sm_mask_enabled(PIO p,uint m,bool e){(void)p;(void)m;(void)e;}
uint pio_get_dreq(PIO p,uint sm,bool tx){(void)p;(void)sm;(void)tx;return 0u;}
bool pio_sm_is_rx_fifo_empty(PIO p,uint sm){(void)p;(void)sm;return true;}
uint32_t pio_sm_get(PIO p,uint sm){(void)p;(void)sm;return 0u;}
uint32_t clock_get_hz(int c){(void)c;return 125000000u;}

int main(void) {
    t425_link_t l[4] = {0};
    for (int i=0;i<4;i++) { l[i].pio=test_pio; l[i].sm_tx=(uint)i; l[i].sm_rx=(uint)i; l[i].tx_dma=1; }
    assert(!l[0].tx_waiting_ack);
    assert(t425_link_send_frame(&l[0],0x11u));
    assert(l[0].tx_waiting_ack);
    assert(!t425_link_send_frame(&l[0],0x22u));
    assert(dma_start_count == 1);
    assert(t425_link_send_frame(&l[1],0x33u));
    assert(l[1].tx_waiting_ack);
    assert(t425_link_send_ack(&l[0]));
    assert(dma_start_count == 3);
    t425_link_note_ack_received(&l[0]);
    assert(!l[0].tx_waiting_ack);
    assert(t425_link_send_frame(&l[0],0x44u));
    assert(l[0].tx_waiting_ack);
    t425_link_stop(&l[0]);
    assert(!l[0].tx_waiting_ack);
    l[0].tx_waiting_ack = true;
    t425_link_dma_deinit(&l[0]);
    assert(!l[0].tx_waiting_ack);
    puts("F_PHY-69 per-byte TX ACK gating: PASS");
    return 0;
}
