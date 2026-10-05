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

static bool core0_read_system_inputs(
    void *context,
    t425_core_phy_system_inputs_t *inputs)
{
    t425_core_phy_ipc_t *ipc = context;
    if (ipc == 0 || !ipc->initialized || inputs == 0)
        return false;
    *inputs = ipc->system_inputs;
    return true;
}

static bool core0_write_system_outputs(
    void *context,
    const t425_core_phy_system_outputs_t *outputs)
{
    t425_core_phy_ipc_t *ipc = context;
    if (ipc == 0 || !ipc->initialized || outputs == 0)
        return false;
    ipc->system_outputs = *outputs;
    return true;
}

static uint32_t admission_pack(uint32_t generation, bool enabled)
{
    generation &= 0x7FFFFFFFu;
    if (generation == 0u)
        generation = 1u;
    return (generation << 1u) | (enabled ? 1u : 0u);
}

static int core0_set_rx_admission(
    void *context,
    uint32_t endpoint_id,
    bool enabled)
{
    t425_core_phy_ipc_t *ipc = context;
    if (ipc == 0 || !ipc->initialized ||
        endpoint_id >= T425_CORE_PHY_MAX_ENDPOINTS)
        return -1;
    __atomic_store_n(&ipc->rx_admission_word[endpoint_id],
                     admission_pack(1u, enabled), __ATOMIC_RELEASE);
    return 0;
}

static int core0_set_rx_admission_credit(
    void *context,
    uint32_t endpoint_id,
    uint32_t generation,
    bool enabled)
{
    t425_core_phy_ipc_t *ipc = context;
    if (ipc == 0 || !ipc->initialized ||
        endpoint_id >= T425_CORE_PHY_MAX_ENDPOINTS)
        return -1;
    __atomic_store_n(&ipc->rx_admission_word[endpoint_id],
                     admission_pack(generation, enabled), __ATOMIC_RELEASE);
    return 0;
}

static int core0_set_rx_arbitration(void *context, bool wait_control)
{
    t425_core_phy_ipc_t *ipc = context;
    if (ipc == 0 || !ipc->initialized)
        return -1;
    __atomic_store_n(&ipc->rx_wait_control_arbitration,
                     wait_control ? 1u : 0u, __ATOMIC_RELEASE);
    return 0;
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
    __atomic_store_n(&ipc->rx_wait_control_arbitration, 0u, __ATOMIC_RELAXED);
    ipc->system_inputs = (t425_core_phy_system_inputs_t){0};
    ipc->system_outputs = (t425_core_phy_system_outputs_t){0};
    for (uint32_t endpoint = 0u; endpoint < T425_CORE_PHY_MAX_ENDPOINTS; ++endpoint) {
        __atomic_store_n(&ipc->rx_admission_word[endpoint], 0u, __ATOMIC_RELAXED);
    }
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
    ops->read_system_inputs = core0_read_system_inputs;
    ops->write_system_outputs = core0_write_system_outputs;
    ops->set_rx_admission = core0_set_rx_admission;
    ops->set_rx_admission_credit = core0_set_rx_admission_credit;
    ops->set_rx_arbitration = core0_set_rx_arbitration;
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
    service->rx_wait_control_claimed = false;
    service->rx_wait_control_mode = false;
    return true;
}

uint32_t t425_core_phy_ipc_service_poll(
    t425_core_phy_ipc_service_t *service)
{
    uint32_t work = 0u;
    if (service == 0 || service->ipc == 0 ||
        !service->ipc->initialized || !service->phy.attached)
        return 0u;

    bool wait_control = __atomic_load_n(
        &service->ipc->rx_wait_control_arbitration, __ATOMIC_ACQUIRE) != 0u;
    if (!wait_control || !service->rx_wait_control_mode)
        service->rx_wait_control_claimed = false;
    service->rx_wait_control_mode = wait_control;

    uint32_t wait_control_winner = T425_CORE_PHY_MAX_ENDPOINTS;
    if (wait_control && !service->rx_wait_control_claimed) {
        for (uint32_t endpoint = 0u;
             endpoint < T425_CORE_PHY_MAX_ENDPOINTS;
             ++endpoint) {
            uint32_t word = __atomic_load_n(
                &service->ipc->rx_admission_word[endpoint],
                __ATOMIC_ACQUIRE);
            bool enabled = (word & 1u) != 0u;
            if (enabled && t425_core_phy_rx_available(&service->phy, endpoint)) {
                wait_control_winner = endpoint;
                break;
            }
        }
    }

    for (uint32_t endpoint = 0u;
         endpoint < T425_CORE_PHY_MAX_ENDPOINTS;
         ++endpoint) {
        t425_core_phy_ipc_byte_t message;
        uint32_t word = __atomic_load_n(
            &service->ipc->rx_admission_word[endpoint], __ATOMIC_ACQUIRE);
        uint32_t generation = (word >> 1u) & 0x7FFFFFFFu;
        bool enabled = (word & 1u) != 0u;

        if (service->phy.ops.set_rx_admission_credit != 0)
            (void)t425_core_phy_set_rx_admission_credit(
                &service->phy, endpoint, generation, enabled);

        /* Keep the descriptor queued when the PHY reports backpressure. */
        if (queue_try_peek(&service->ipc->core0_to_core1[endpoint], &message) &&
            t425_core_phy_tx(&service->phy, endpoint, message.byte) == 0) {
            (void)queue_try_remove(
                &service->ipc->core0_to_core1[endpoint], &message);
            ++work;
        }

        /*
         * Poll RX even when the Core0 mailbox is full so the PHY can
         * capture and retain one physical DATA byte without ACKing it.
         */
        {
            bool available = t425_core_phy_rx_available(
                &service->phy, endpoint);
            if (wait_control &&
                (service->rx_wait_control_claimed ||
                 (wait_control_winner != T425_CORE_PHY_MAX_ENDPOINTS &&
                  wait_control_winner != endpoint)))
                available = false;
            if (!queue_is_full(&service->ipc->core1_to_core0[endpoint]) &&
                available) {
            uint32_t received;
            if (t425_core_phy_rx(&service->phy, endpoint, &received) == 0) {
                message.byte = (uint8_t)(received & 0xFFu);
                if (queue_try_add(
                        &service->ipc->core1_to_core0[endpoint], &message)) {
                    ++work;
                    if (wait_control)
                        service->rx_wait_control_claimed = true;
                }
            }
        }
        }
    }

    if (service->phy.ops.read_system_inputs != 0)
        (void)service->phy.ops.read_system_inputs(
            service->phy.ops.context, &service->ipc->system_inputs);
    if (service->phy.ops.write_system_outputs != 0)
        (void)service->phy.ops.write_system_outputs(
            service->phy.ops.context, &service->ipc->system_outputs);
    return work;
}
