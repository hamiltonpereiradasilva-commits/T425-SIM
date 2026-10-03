#include "t425_core_phy_ipc.h"

static int core0_tx(void *context, uint32_t endpoint_id, uint16_t frame)
{
    t425_core_phy_ipc_t *ipc = context;
    t425_core_phy_ipc_byte_t message;

    if (ipc == 0 || !ipc->initialized ||
        endpoint_id >= T425_CORE_PHY_MAX_ENDPOINTS || frame > 0xFFu)
        return -1;

    message.byte = (uint8_t)frame;
    return queue_try_add(&ipc->core0_to_core1[endpoint_id], &message) ? 0 : -2;
}

static bool core0_rx_available(void *context, uint32_t endpoint_id)
{
    t425_core_phy_ipc_t *ipc = context;
    return ipc != 0 && ipc->initialized &&
           endpoint_id < T425_CORE_PHY_MAX_ENDPOINTS &&
           !queue_is_empty(&ipc->core1_to_core0[endpoint_id]);
}

static int core0_rx_get(
    void *context,
    uint32_t endpoint_id,
    uint32_t *frame)
{
    t425_core_phy_ipc_t *ipc = context;
    t425_core_phy_ipc_byte_t message;

    if (ipc == 0 || !ipc->initialized || frame == 0 ||
        endpoint_id >= T425_CORE_PHY_MAX_ENDPOINTS)
        return -1;
    if (!queue_try_remove(&ipc->core1_to_core0[endpoint_id], &message))
        return -2;

    *frame = message.byte;
    return 0;
}

bool t425_core_phy_ipc_init(t425_core_phy_ipc_t *ipc)
{
    uint32_t initialized_queues = 0u;
    if (ipc == 0)
        return false;

    ipc->initialized = false;
    for (uint32_t endpoint = 0u;
         endpoint < T425_CORE_PHY_MAX_ENDPOINTS;
         ++endpoint) {
        if (!queue_init(&ipc->core0_to_core1[endpoint],
                        sizeof(t425_core_phy_ipc_byte_t),
                        T425_CORE_PHY_IPC_QUEUE_DEPTH))
            goto fail;
        ++initialized_queues;
        if (!queue_init(&ipc->core1_to_core0[endpoint],
                        sizeof(t425_core_phy_ipc_byte_t),
                        T425_CORE_PHY_IPC_QUEUE_DEPTH))
            goto fail;
        ++initialized_queues;
    }
    ipc->initialized = true;
    return true;

fail:
    for (uint32_t endpoint = 0u;
         endpoint < T425_CORE_PHY_MAX_ENDPOINTS;
         ++endpoint) {
        if (initialized_queues > 0u) {
            queue_free(&ipc->core0_to_core1[endpoint]);
            --initialized_queues;
        }
        if (initialized_queues > 0u) {
            queue_free(&ipc->core1_to_core0[endpoint]);
            --initialized_queues;
        }
    }
    return false;
}

void t425_core_phy_ipc_make_core0_ops(
    t425_core_phy_ipc_t *ipc,
    t425_core_phy_ops_t *ops)
{
    if (ipc == 0 || ops == 0)
        return;

    ops->tx_frame = core0_tx;
    ops->rx_available = core0_rx_available;
    ops->rx_get = core0_rx_get;
    ops->context = ipc;
}

bool t425_core_phy_ipc_service_init(
    t425_core_phy_ipc_service_t *service,
    t425_core_phy_ipc_t *ipc,
    const t425_core_phy_ops_t *core1_phy_ops)
{
    if (service == 0 || ipc == 0 || !ipc->initialized ||
        core1_phy_ops == 0)
        return false;

    if (!t425_core_phy_init(
            &service->phy,
            core1_phy_ops,
            T425_CORE_PHY_MAX_ENDPOINTS))
        return false;

    service->ipc = ipc;
    return true;
}

uint32_t t425_core_phy_ipc_service_poll(
    t425_core_phy_ipc_service_t *service)
{
    uint32_t work = 0u;
    if (service == 0 || service->ipc == 0 ||
        !service->ipc->initialized || !service->phy.attached)
        return 0u;

    for (uint32_t endpoint = 0u;
         endpoint < T425_CORE_PHY_MAX_ENDPOINTS;
         ++endpoint) {
        t425_core_phy_ipc_byte_t message;

        /* Keep the descriptor queued when the PHY reports backpressure. */
        if (queue_try_peek(&service->ipc->core0_to_core1[endpoint], &message) &&
            t425_core_phy_tx(&service->phy, endpoint, message.byte) == 0) {
            (void)queue_try_remove(
                &service->ipc->core0_to_core1[endpoint], &message);
            ++work;
        }

        /* Do not consume PHY RX data when the Core0 mailbox is full. */
        if (!queue_is_full(&service->ipc->core1_to_core0[endpoint]) &&
            t425_core_phy_rx_available(&service->phy, endpoint)) {
            uint32_t received;
            if (t425_core_phy_rx(&service->phy, endpoint, &received) == 0) {
                message.byte = (uint8_t)(received & 0xFFu);
                if (queue_try_add(
                        &service->ipc->core1_to_core0[endpoint], &message))
                    ++work;
            }
        }
    }
    return work;
}
