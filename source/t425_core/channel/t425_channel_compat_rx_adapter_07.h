#ifndef T425_CHANNEL_COMPAT_RX_ADAPTER_07_H
#define T425_CHANNEL_COMPAT_RX_ADAPTER_07_H

#include <stdbool.h>
#include <stdint.h>

#include "t425_channel_compat_input.h"
#include "t425_channel_dma_rx_adapter_10.h"
#include "t425_core.h"

#define T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS 4u

typedef enum {
    T425_CHANNEL_COMPAT_RX_07_INVALID = -1,
    T425_CHANNEL_COMPAT_RX_07_NO_ACTION = 0,
    T425_CHANNEL_COMPAT_RX_07_NO_DATA,
    T425_CHANNEL_COMPAT_RX_07_RETAINED,
    T425_CHANNEL_COMPAT_RX_07_ACCEPTED,
    T425_CHANNEL_COMPAT_RX_07_STALE,
    T425_CHANNEL_COMPAT_RX_07_DATA_CONSUMED,
    T425_CHANNEL_COMPAT_RX_07_ACK_CLEARED,
    T425_CHANNEL_COMPAT_RX_07_COMMIT_RETRY,
    T425_CHANNEL_COMPAT_RX_07_COMMIT_SUCCESS,
    T425_CHANNEL_COMPAT_RX_07_COMMIT_ERROR,
    T425_CHANNEL_COMPAT_RX_07_POST_WRITE,
    T425_CHANNEL_COMPAT_RX_07_COMPLETE
} t425_channel_compat_rx_07_result_t;

typedef struct {
    uint8_t bound;
    uint8_t held_valid;
    uint32_t held_frame;
    t425_channel_compat_input_key_t key;
} t425_channel_compat_rx_07_endpoint_t;

typedef struct t425_channel_compat_rx_adapter_07 {
    t425_core_phy_t *phy;
    t425_channel_compat_input_t *input;
    t425_channel_dma_memory_04_t *f04;
    uint8_t lane_id[4];
    t425_channel_compat_rx_07_endpoint_t endpoint[
        T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS];
} t425_channel_compat_rx_adapter_07_t;

typedef enum {
    T425_CHANNEL_RX_DISPATCH_07_LEGACY_F05 = 0,
    T425_CHANNEL_RX_DISPATCH_07_COMPAT_INPUT
} t425_channel_rx_dispatch_07_mode_t;

typedef struct {
    t425_channel_rx_dispatch_07_mode_t mode;
    t425_core_t *core;
    t425_channel_dma_rx_adapter_10_t *legacy;
    t425_channel_compat_rx_adapter_07_t *compat;
} t425_channel_rx_dispatch_07_t;

bool t425_channel_compat_rx_adapter_07_init(
    t425_channel_compat_rx_adapter_07_t *adapter,
    t425_core_phy_t *phy, t425_channel_compat_input_t *input,
    t425_channel_dma_memory_04_t *f04);

/* F123 sideband hook for the IF Next boundary: binds the unique eligible
 * INPUT latch owner without changing the MIR/CROM interface. */
t425_channel_ifnext_owner_result_t
t425_channel_compat_rx_adapter_07_bind_ifnext_owner(
    t425_channel_compat_rx_adapter_07_t *adapter, bool if_next,
    t425_channel_compat_input_key_t *selected_owner);

/* lane_id is explicit test/config metadata; it is not derived from endpoint,
 * physical_link, architectural address, or the F52 DMA slot. */
bool t425_channel_compat_rx_adapter_07_adopt_active(
    t425_channel_compat_rx_adapter_07_t *adapter, const t425_core_t *core,
    uint32_t architectural_channel, uint32_t request_id,
    uint32_t operation_generation, uint8_t endpoint,
    uint8_t physical_link, uint8_t lane_id);

/* Retire only a fully committed ACTIVE/COMPLETED INPUT; preserves held RX. */
bool t425_channel_compat_rx_adapter_07_retire_completed(
    t425_channel_compat_rx_adapter_07_t *adapter, t425_core_t *core,
    uint32_t endpoint);

/* Reads at most one F10 transport frame and releases it only on ACCEPTED. */
t425_channel_compat_rx_07_result_t
 t425_channel_compat_rx_adapter_07_poll_endpoint(
    t425_channel_compat_rx_adapter_07_t *adapter, const t425_core_t *core,
    uint32_t endpoint);

/* Advances exactly one CHANNEL state transition or one F04 transaction. */
t425_channel_compat_rx_07_result_t
 t425_channel_compat_rx_adapter_07_step_endpoint(
    t425_channel_compat_rx_adapter_07_t *adapter, t425_core_t *core,
    uint32_t endpoint);

/* Immutable per-test selector: each poll invokes either F10/F05 or compat. */
bool t425_channel_rx_dispatch_07_init(
    t425_channel_rx_dispatch_07_t *dispatch,
    t425_channel_rx_dispatch_07_mode_t mode, t425_core_t *core,
    t425_channel_dma_rx_adapter_10_t *legacy,
    t425_channel_compat_rx_adapter_07_t *compat);

t425_channel_compat_rx_07_result_t t425_channel_rx_dispatch_07_poll(
    t425_channel_rx_dispatch_07_t *dispatch, uint32_t endpoint);

t425_channel_compat_rx_07_result_t t425_channel_rx_dispatch_07_step(
    t425_channel_rx_dispatch_07_t *dispatch, uint32_t endpoint);

#endif
