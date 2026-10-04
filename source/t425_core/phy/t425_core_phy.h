#ifndef T425_CORE_PHY_H
#define T425_CORE_PHY_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define T425_CORE_PHY_MAX_ENDPOINTS 4u
#define T425_CORE_PHY_MAX_LINKS      4u

typedef int (*t425_core_phy_tx_fn)(
    void *context,
    uint32_t endpoint_id,
    uint16_t frame);

typedef bool (*t425_core_phy_rx_available_fn)(
    void *context,
    uint32_t endpoint_id);

typedef int (*t425_core_phy_rx_get_fn)(
    void *context,
    uint32_t endpoint_id,
    uint32_t *frame);

typedef struct
{
    t425_core_phy_tx_fn tx_frame;
    t425_core_phy_rx_available_fn rx_available;
    t425_core_phy_rx_get_fn rx_get;
    void *context;
} t425_core_phy_ops_t;

typedef struct
{
    t425_core_phy_ops_t ops;
    uint32_t endpoint_count;
    bool attached;
} t425_core_phy_t;

bool t425_core_phy_init(
    t425_core_phy_t *phy,
    const t425_core_phy_ops_t *ops,
    uint32_t endpoint_count);

int t425_core_phy_tx(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint16_t frame);

bool t425_core_phy_rx_available(
    t425_core_phy_t *phy,
    uint32_t endpoint_id);

int t425_core_phy_rx(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint32_t *frame);

#ifdef __cplusplus
}
#endif

#endif
