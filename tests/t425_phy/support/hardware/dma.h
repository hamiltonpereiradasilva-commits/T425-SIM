#ifndef T425_TEST_DMA_H
#define T425_TEST_DMA_H
#include <stdbool.h>
#include <stdint.h>
typedef unsigned int uint;
typedef struct { uint size; bool read_inc; bool write_inc; uint dreq; } dma_channel_config;
#define DMA_SIZE_32 2u
extern int test_dma_fail_claim_number;
extern int test_dma_claim_count;
extern int test_dma_abort_count;
extern int test_dma_unclaim_count;
int dma_claim_unused_channel(bool required);
void dma_channel_unclaim(uint channel);
void dma_channel_abort(uint channel);
dma_channel_config dma_channel_get_default_config(uint channel);
void channel_config_set_transfer_data_size(dma_channel_config *c, uint size);
void channel_config_set_read_increment(dma_channel_config *c, bool value);
void channel_config_set_write_increment(dma_channel_config *c, bool value);
void channel_config_set_dreq(dma_channel_config *c, uint value);
void dma_channel_configure(uint channel, const dma_channel_config *c,
                           volatile void *write_addr, const volatile void *read_addr,
                           uint32_t count, bool start);
void dma_channel_set_read_addr(uint channel, const volatile void *read_addr, bool trigger);
void dma_channel_set_trans_count(uint channel, uint32_t count, bool trigger);
#endif
