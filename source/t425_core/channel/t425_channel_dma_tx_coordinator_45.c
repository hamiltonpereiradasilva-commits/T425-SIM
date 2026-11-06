#include "t425_channel_dma_tx_coordinator_45.h"

#include <string.h>
#include "t425_channel_route_config_52_table.h"

bool t425_channel_dma_tx_coordinator_45_init(
    t425_channel_dma_tx_coordinator_45_t *coordinator,
    t425_core_t *core, t425_channel_dma_ack_adapter_09_t *ack_adapter,
    t425_core_phy_ipc_t *ipc)
{
    if (coordinator == NULL || core == NULL || ack_adapter == NULL ||
        ipc == NULL || !ipc->initialized || ack_adapter->ipc != ipc ||
        ack_adapter->dma != &core->channel_dma_shift)
        return false;
    memset(coordinator, 0, sizeof(*coordinator));
    coordinator->core = core;
    coordinator->ack_adapter = ack_adapter;
    coordinator->ipc = ipc;
    return true;
}

bool t425_channel_dma_tx_coordinator_45_bind_active(
    t425_channel_dma_tx_coordinator_45_t *coordinator,
    uint32_t architectural_channel, uint32_t request_id,
    uint32_t operation_generation, uint8_t endpoint,
    uint8_t physical_link, uint8_t dma_slot,
    const t425_core_phy_tx_lease_t *lease)
{
    t425_channel_admission_request_21_t snapshot;
    t425_channel_dma_tx_coord_45_session_t *session;
    if (coordinator == NULL || coordinator->core == NULL ||
        coordinator->ack_adapter == NULL || endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        physical_link >= T425_CORE_PHY_MAX_ENDPOINTS || dma_slot >= 4u ||
        coordinator->servicing || lease == NULL ||
        !t425_core_phy_ipc_tx_lease_owned(lease) || lease->ipc != coordinator->ipc ||
        lease->endpoint_id != endpoint)
        return false;
    session = &coordinator->session[endpoint];
    if (session->valid || session->releasing ||
        (session->retired && session->retired_request_id == request_id &&
         session->retired_operation_generation == operation_generation) ||
        !coordinator->ack_adapter->endpoint_mapped[endpoint] ||
        coordinator->ack_adapter->endpoint_to_tx_slot[endpoint] != dma_slot ||
        coordinator->ack_adapter->pending[endpoint].valid ||
        coordinator->ack_adapter->reserved[endpoint].valid ||
        coordinator->ack_adapter->held[endpoint].valid ||
        !t425_core_channel_admission_get_active(
            coordinator->core, architectural_channel, request_id,
            operation_generation, endpoint, physical_link, &snapshot) ||
        snapshot.direction != T425_CHANNEL_DMA_TX ||
        snapshot.lane != dma_slot)
        return false;
    for (uint32_t ep = 0u; ep < T425_CORE_PHY_MAX_ENDPOINTS; ++ep) {
        if (ep != endpoint && coordinator->session[ep].valid &&
            coordinator->session[ep].dma_slot == dma_slot)
            return false;
    }
    if (!t425_channel_dma_ack_adapter_09_bind_lease(
            coordinator->ack_adapter, lease))
        return false;
    session->session = snapshot;
    session->dma_slot = dma_slot;
    session->lease = *lease;
    session->releasing = false;
    session->valid = true;
    return true;
}

static void cancel_reservations(
    t425_core_phy_ipc_tx_reservation_t *ipc_reservation,
    t425_channel_dma_ack_09_reservation_t *ack_reservation,
    t425_channel_dma_05_tx_reservation_t *f05_reservation)
{
    t425_channel_dma_ack_adapter_09_cancel_submission(ack_reservation);
    t425_channel_dma_shift_05_tx_submission_cancel(f05_reservation);
    t425_core_phy_ipc_tx_cancel(ipc_reservation);
}

t425_channel_dma_tx_coord_45_result_t
t425_channel_dma_tx_coordinator_45_submit_one(
    t425_channel_dma_tx_coordinator_45_t *coordinator, uint8_t endpoint)
{
    t425_channel_dma_tx_coord_45_session_t *session;
    t425_core_phy_ipc_tx_reservation_t ipc_reservation = {0};
    t425_channel_dma_ack_09_reservation_t ack_reservation = {0};
    t425_channel_dma_05_tx_reservation_t f05_reservation = {0};
    t425_channel_dma_05_status_t prep;
    uint8_t byte;
    uint64_t ticket;
    if (coordinator == NULL || coordinator->core == NULL ||
        endpoint >= T425_CORE_PHY_MAX_ENDPOINTS)
        return T425_CHANNEL_DMA_TX_COORD_45_INVALID;
    if (coordinator->servicing)
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    coordinator->servicing = true;
    session = &coordinator->session[endpoint];
    if (!session->valid || session->releasing ||
        !t425_core_phy_ipc_tx_lease_owned(&session->lease) ||
        coordinator->ack_adapter->pending[endpoint].valid ||
        coordinator->ack_adapter->reserved[endpoint].valid) {
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    }
    if (t425_channel_dma_shift_05_tx_complete(
            &coordinator->core->channel_dma_shift, session->dma_slot)) {
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_COMPLETE;
    }
    prep = t425_channel_dma_shift_05_tx_prepare(
        &coordinator->core->channel_dma_shift, session->dma_slot,
        &byte, &ticket);
    if (prep == T425_CHANNEL_DMA_05_RETRY) {
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_RETRY;
    }
    if (prep == T425_CHANNEL_DMA_05_BUSY) {
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    }
    if (prep != T425_CHANNEL_DMA_05_OK || ticket == 0u) {
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_INVALID;
    }
    if (!t425_core_phy_ipc_tx_reserve(
            coordinator->ipc, &session->lease, endpoint, byte,
            &ipc_reservation)) {
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    }
    if (!t425_channel_dma_ack_adapter_09_reserve_submission(
            coordinator->ack_adapter, &session->session, session->dma_slot,
            ticket, byte, ipc_reservation.generation, &ack_reservation) ||
        !t425_channel_dma_shift_05_tx_submission_reserve(
            &coordinator->core->channel_dma_shift, session->dma_slot,
            ticket, byte, &f05_reservation)) {
        cancel_reservations(&ipc_reservation, &ack_reservation,
                            &f05_reservation);
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    }

    /* Final read-only preflight. No fallible work/callback follows commit. */
    if (!t425_core_phy_ipc_tx_reservation_valid(&ipc_reservation) ||
        !t425_channel_dma_shift_05_tx_submission_reservation_valid(
            &f05_reservation) ||
        !t425_channel_dma_ack_adapter_09_reservation_valid(&ack_reservation)) {
        cancel_reservations(&ipc_reservation, &ack_reservation,
                            &f05_reservation);
        coordinator->servicing = false;
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    }
    t425_channel_dma_shift_05_tx_submission_commit(&f05_reservation);
    t425_channel_dma_ack_adapter_09_commit_submission(&ack_reservation);
    t425_core_phy_ipc_tx_publish_reserved(&ipc_reservation);
    coordinator->servicing = false;
    return T425_CHANNEL_DMA_TX_COORD_45_ACCEPTED;
}

t425_channel_dma_tx_coord_45_result_t
t425_channel_dma_tx_coordinator_45_poll_ack(
    t425_channel_dma_tx_coordinator_45_t *coordinator, uint8_t endpoint)
{
    t425_channel_dma_ack_09_result_t result;
    if (coordinator == NULL || coordinator->ack_adapter == NULL ||
        endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        !coordinator->session[endpoint].valid)
        return T425_CHANNEL_DMA_TX_COORD_45_INVALID;
    if (coordinator->servicing)
        return T425_CHANNEL_DMA_TX_COORD_45_BUSY;
    coordinator->servicing = true;
    result = t425_channel_dma_ack_adapter_09_poll_endpoint(
        coordinator->ack_adapter, endpoint);
    coordinator->servicing = false;
    if (result == T425_CHANNEL_DMA_ACK_09_APPLIED) {
        const t425_channel_dma_tx_coord_45_session_t *session =
            &coordinator->session[endpoint];
        const t425_channel_route_52_t *route = NULL;
        for (size_t i = 0u; i < T425_CHANNEL_ROUTE_52_COUNT; ++i) {
            const t425_channel_route_52_t *candidate =
                &t425_channel_routes_52[i];
            if (candidate->architectural_channel ==
                    session->session.architectural_channel) {
                route = candidate;
                break;
            }
        }
        /* Set after validation/commit; architectural route selects the slot. */
        if (route != NULL && route->direction == T425_CHANNEL_DMA_TX &&
            session->session.direction == T425_CHANNEL_DMA_TX &&
            route->endpoint == endpoint && route->dma_slot == session->dma_slot &&
            route->dma_slot == session->session.lane &&
            route->physical_link == session->session.physical_link &&
            coordinator->core->t425_channel_dma.tx[route->dma_slot].count_reg == 0u)
            (void)channel_vbc_output_terminal_ack(
                &coordinator->core->channel_vbc_state, route->dma_slot,
                coordinator->core->t425_channel_dma.tx[route->dma_slot].count_reg);
        return T425_CHANNEL_DMA_TX_COORD_45_ACK_APPLIED;
    }
    if (result == T425_CHANNEL_DMA_ACK_09_RETAINED ||
        result == T425_CHANNEL_DMA_ACK_09_NO_EVENT)
        return T425_CHANNEL_DMA_TX_COORD_45_ACK_PENDING;
    return T425_CHANNEL_DMA_TX_COORD_45_INVALID;
}

bool t425_channel_dma_tx_coordinator_45_complete(
    const t425_channel_dma_tx_coordinator_45_t *coordinator,
    uint8_t endpoint)
{
    const t425_channel_dma_tx_coord_45_session_t *session;
    if (coordinator == NULL || coordinator->core == NULL ||
        endpoint >= T425_CORE_PHY_MAX_ENDPOINTS)
        return false;
    session = &coordinator->session[endpoint];
    return session->valid && !coordinator->ack_adapter->pending[endpoint].valid &&
        !coordinator->ack_adapter->reserved[endpoint].valid &&
        t425_channel_dma_shift_05_tx_complete(
            &coordinator->core->channel_dma_shift, session->dma_slot);
}

bool t425_channel_dma_tx_coordinator_45_release_request(
    t425_channel_dma_tx_coordinator_45_t *coordinator, uint8_t endpoint)
{
    t425_channel_dma_tx_coord_45_session_t *session;
    if (coordinator == NULL || endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        coordinator->servicing)
        return false;
    session = &coordinator->session[endpoint];
    if (!session->valid || session->releasing ||
        !t425_channel_dma_tx_coordinator_45_complete(coordinator, endpoint) ||
        coordinator->ack_adapter->pending[endpoint].valid ||
        coordinator->ack_adapter->reserved[endpoint].valid ||
        coordinator->ack_adapter->held[endpoint].valid)
        return false;
    if (!t425_core_phy_ipc_tx_lease_release_request(&session->lease))
        return false;
    session->releasing = true;
    return true;
}

bool t425_channel_dma_tx_coordinator_45_poll_release(
    t425_channel_dma_tx_coordinator_45_t *coordinator, uint8_t endpoint)
{
    t425_channel_dma_tx_coord_45_session_t *session;
    if (coordinator == NULL || endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        coordinator->servicing)
        return false;
    session = &coordinator->session[endpoint];
    if (!session->valid || !session->releasing ||
        !t425_core_phy_ipc_tx_lease_poll_release(&session->lease))
        return false;
    session->retired = true;
    session->retired_request_id = session->session.request_id;
    session->retired_operation_generation =
        session->session.operation_generation;
    session->valid = false;
    session->releasing = false;
    coordinator->ack_adapter->tx_lease[endpoint] =
        (t425_core_phy_tx_lease_t){0};
    return true;
}
