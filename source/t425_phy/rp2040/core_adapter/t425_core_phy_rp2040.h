#ifndef T425_CORE_PHY_RP2040_H
#define T425_CORE_PHY_RP2040_H

#include <stdbool.h>
#include <stdint.h>

#include "t425_core_phy.h"
#include "t425_link.h"

#define T425_CORE_PHY_ENDPOINT_COUNT T425_CORE_PHY_MAX_LINKS
#define T425_CORE_PHY_PHYSICAL_LINK_COUNT T425_CORE_PHY_MAX_LINKS

typedef struct
{
    /* Logical T425 endpoint ID; never t425_channel_select. */
    uint8_t endpoint_id;
    /* Physical Link0..Link3 index into links[]. */
    uint8_t physical_link_id;
} t425_core_phy_route_t;

typedef struct
{
    t425_link_t *links[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT];
    uint8_t route_count;
    uint32_t rx_data[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    uint32_t physical_ack_count[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    bool rx_data_pending[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    bool rx_data_consumed[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    bool rx_admission[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    uint32_t rx_grant_generation[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    uint32_t rx_consumed_generation[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    bool physical_ack_pending[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    bool ack_irq_registered;
} t425_core_phy_rp2040_t;

/*
 * Bind the four physical link instances and an explicit one-to-one
 * logical-endpoint -> physical-link route table. The caller owns the
 * table and supplies all four entries; no identity mapping is implied.
 */
bool t425_core_phy_rp2040_init(
    t425_core_phy_rp2040_t *adapter,
    t425_link_t *const links[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    const t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT]);

/*
 * Initialize Link0..Link3 from the P1.6 board map: GPIO pairs 0/1,
 * 2/3, 4/5, 6/7; two links per PIO block; explicit route table required.
 * Call from Core1 before t425_core_phy_rp2040_start().
 */
bool t425_core_phy_rp2040_init_four_links(
    t425_core_phy_rp2040_t *adapter,
    t425_link_t links[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    uint32_t *const rx_buffers[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    uint32_t rx_buffer_words,
    const t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT]);

/*
 * Fill callbacks for the PHY-side endpoint service. These callbacks access
 * PIO and must execute on RP2040 Core 1. Core 0 must use the mailbox-backed
 * operations from t425_core_phy_ipc instead of calling this adapter.
 */
void t425_core_phy_rp2040_make_ops(
    t425_core_phy_rp2040_t *adapter,
    t425_core_phy_ops_t *ops);

/* Core1 lifecycle. start() enables the private overlapped-ACK IRQ path. */
/* Core1 poll hook for deferred physical ACKs and TX arbitration. */
void t425_core_phy_rp2040_service(t425_core_phy_rp2040_t *adapter);

bool t425_core_phy_rp2040_start(t425_core_phy_rp2040_t *adapter);
void t425_core_phy_rp2040_stop(t425_core_phy_rp2040_t *adapter);

#endif
