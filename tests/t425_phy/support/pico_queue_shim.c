#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

static test_pio_hw_t test_pio_hw;
PIO test_pio = &test_pio_hw;
int test_dma_fail_claim_number = -1;
int test_dma_claim_count = 0;
int test_dma_abort_count = 0;
int test_dma_unclaim_count = 0;

uint pio_add_program(PIO pio, const pio_program *program)
{
    (void)pio;
    (void)program;
    return 0u;
}
void pio_gpio_init(PIO pio, uint pin) { (void)pio; (void)pin; }
void pio_sm_set_consecutive_pindirs(PIO pio, uint sm, uint pin, uint count, bool out)
{ (void)pio; (void)sm; (void)pin; (void)count; (void)out; }
void pio_sm_init(PIO pio, uint sm, uint offset, const pio_sm_config *config)
{ (void)pio; (void)sm; (void)offset; (void)config; }
void pio_enable_sm_mask_in_sync(PIO pio, uint mask) { (void)pio; (void)mask; }
void pio_set_sm_mask_enabled(PIO pio, uint mask, bool enabled)
{ (void)pio; (void)mask; (void)enabled; }
uint pio_get_dreq(PIO pio, uint sm, bool is_tx)
{ (void)pio; (void)sm; (void)is_tx; return 0u; }
bool pio_sm_is_rx_fifo_empty(PIO pio, uint sm)
{ (void)pio; (void)sm; return true; }
uint32_t pio_sm_get(PIO pio, uint sm)
{ (void)pio; (void)sm; return 0u; }

int dma_claim_unused_channel(bool required)
{
    (void)required;
    ++test_dma_claim_count;
    if (test_dma_fail_claim_number == test_dma_claim_count)
        return -1;
    return test_dma_claim_count - 1;
}
void dma_channel_unclaim(uint channel) { (void)channel; ++test_dma_unclaim_count; }
void dma_channel_abort(uint channel) { (void)channel; ++test_dma_abort_count; }
dma_channel_config dma_channel_get_default_config(uint channel)
{ dma_channel_config c = {0u, false, false, channel}; return c; }
void channel_config_set_transfer_data_size(dma_channel_config *c, uint size)
{ c->size = size; }
void channel_config_set_read_increment(dma_channel_config *c, bool value)
{ c->read_inc = value; }
void channel_config_set_write_increment(dma_channel_config *c, bool value)
{ c->write_inc = value; }
void channel_config_set_dreq(dma_channel_config *c, uint value)
{ c->dreq = value; }
void dma_channel_configure(uint channel, const dma_channel_config *c,
                           volatile void *write_addr, const volatile void *read_addr,
                           uint32_t count, bool start)
{ (void)channel; (void)c; (void)write_addr; (void)read_addr; (void)count; (void)start; }
void dma_channel_set_read_addr(uint channel, const volatile void *read_addr, bool trigger)
{ (void)channel; (void)read_addr; (void)trigger; }
void dma_channel_set_trans_count(uint channel, uint32_t count, bool trigger)
{ (void)channel; (void)count; (void)trigger; }

uint32_t clock_get_hz(int clock) { (void)clock; return 20000000u; }
void gpio_init(uint pin) { (void)pin; }
void gpio_set_dir(uint pin, bool out) { (void)pin; (void)out; }
bool gpio_get(uint pin) { (void)pin; return false; }
