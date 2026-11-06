#include "t425_channel_compat_rx_adapter_07.h"

#include <string.h>

#include "t425_channel_route_config_52_table.h"


static const t425_channel_route_52_t *route_52_find(uint32_t address)
{
    for (size_t i = 0u; i < T425_CHANNEL_ROUTE_52_COUNT; ++i) {
        if (t425_channel_routes_52[i].architectural_channel == address)
            return &t425_channel_routes_52[i];
    }
    return NULL;
}

static const t425_channel_admission_entry_21_t *admission_find(
    const t425_core_t *core, uint32_t address)
{
    if (core == NULL)
        return NULL;
    for (size_t i = 0u; i < T425_CHANNEL_ADMISSION_21_CHANNELS; ++i) {
        if (core->channel_admission.channel[i].architectural_channel == address)
            return &core->channel_admission.channel[i];
    }
    return NULL;
}

static bool f17_matches_compat_lane(
    const t425_channel_compat_rx_adapter_07_t *adapter,
    const t425_channel_compat_rx_07_endpoint_t *endpoint,
    const t425_core_t *core)
{
    const t425_channel_compat_input_lane_t *compat_lane;
    const t425_channel_dma_lane_t *f17;
    uint8_t slot;
    if (adapter == NULL || endpoint == NULL || core == NULL ||
        endpoint->bound == 0u)
        return false;
    slot = endpoint->key.route.stack_lane;
    if (slot >= T425_CHANNEL_DMA_SLOT_COUNT)
        return false;
    compat_lane = &adapter->input->lane[endpoint->key.route.input_lane];
    f17 = &core->t425_channel_dma.rx[slot];
    return f17->count_reg == compat_lane->count_reg &&
        f17->ptr_reg == compat_lane->ptr_reg &&
        f17->dbuff_reg == compat_lane->dbuff_reg;
}

static void publish_f17_compat_lane(
    t425_channel_compat_rx_adapter_07_t *adapter,
    t425_channel_compat_rx_07_endpoint_t *endpoint,
    t425_core_t *core)
{
    t425_channel_compat_input_lane_t *compat_lane =
        &adapter->input->lane[endpoint->key.route.input_lane];
    t425_channel_dma_lane_t *f17 =
        &core->t425_channel_dma.rx[endpoint->key.route.stack_lane];
    f17->count_reg = compat_lane->count_reg;
    f17->ptr_reg = compat_lane->ptr_reg;
    f17->dbuff_reg = compat_lane->dbuff_reg;
}

static bool fetch_active_request(
    const t425_channel_compat_rx_adapter_07_t *adapter,
    const t425_channel_compat_rx_07_endpoint_t *endpoint,
    const t425_core_t *core,
    t425_channel_admission_request_21_t *request)
{
    const t425_channel_compat_input_route_t *route;
    const t425_channel_admission_entry_21_t *entry;
    if (adapter == NULL || endpoint == NULL || core == NULL || request == NULL ||
        endpoint->bound == 0u)
        return false;
    const t425_channel_route_52_t *route52;
    route = &endpoint->key.route;
    entry = admission_find(core, route->architectural_channel);
    route52 = route_52_find(route->architectural_channel);
    if (entry == NULL || route52 == NULL ||
        route52->direction != T425_CHANNEL_DMA_RX ||
        route52->dma_slot != route->stack_lane ||
        route52->endpoint != route->endpoint_id ||
        route52->physical_link != route->physical_link ||
        entry->route_valid == 0u ||
        entry->direction != T425_CHANNEL_DMA_RX ||
        entry->lane != route->stack_lane ||
        entry->endpoint != route->endpoint_id ||
        entry->physical_link != route->physical_link)
        return false;
    return t425_core_channel_admission_get_active_compat_input(
        core, route->architectural_channel,
        endpoint->key.request_id, endpoint->key.generation,
        route->endpoint_id, route->physical_link, request) &&
        request->direction == T425_CHANNEL_DMA_RX &&
        request->lane == route->stack_lane &&
        request->endpoint == route->endpoint_id &&
        request->physical_link == route->physical_link &&
        f17_matches_compat_lane(adapter, endpoint, core);
}

bool t425_channel_compat_rx_adapter_07_init(
    t425_channel_compat_rx_adapter_07_t *adapter,
    t425_core_phy_t *phy, t425_channel_compat_input_t *input,
    t425_channel_dma_memory_04_t *f04)
{
    if (adapter == NULL || phy == NULL || !phy->attached || input == NULL ||
        f04 == NULL || f04->memory == NULL || f04->bank == NULL)
        return false;
    memset(adapter, 0, sizeof(*adapter));
    adapter->phy = phy;
    adapter->input = input;
    adapter->f04 = f04;
    return true;
}

t425_channel_ifnext_owner_result_t
t425_channel_compat_rx_adapter_07_bind_ifnext_owner(
    t425_channel_compat_rx_adapter_07_t *adapter, bool if_next,
    t425_channel_compat_input_key_t *selected_owner)
{
    if (adapter == NULL || adapter->input == NULL)
        return T425_CHANNEL_IFNEXT_OWNER_NONE;
    return t425_channel_compat_input_bind_unique_ifnext_owner(
        adapter->input, if_next, selected_owner);
}

bool t425_channel_compat_rx_adapter_07_adopt_active(
    t425_channel_compat_rx_adapter_07_t *adapter, const t425_core_t *core,
    uint32_t architectural_channel, uint32_t request_id,
    uint32_t operation_generation, uint8_t endpoint_id,
    uint8_t physical_link, uint8_t lane_id)
{
    t425_channel_admission_request_21_t request;
    t425_channel_compat_input_route_t route;
    t425_channel_compat_input_key_t key;
    t425_channel_compat_rx_07_endpoint_t *bound;
    const t425_channel_admission_entry_21_t *entry;
    const t425_channel_route_52_t *route52;
    const t425_channel_dma_lane_t *f17;

    if (adapter == NULL || adapter->phy == NULL || adapter->input == NULL ||
        core == NULL || endpoint_id >= adapter->phy->endpoint_count ||
        endpoint_id >= T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS ||
        request_id == 0u || operation_generation == 0u)
        return false;
    entry = admission_find(core, architectural_channel);
    route52 = route_52_find(architectural_channel);
    if (entry == NULL || route52 == NULL ||
        route52->direction != T425_CHANNEL_DMA_RX ||
        route52->endpoint != endpoint_id ||
        route52->physical_link != physical_link ||
        entry->route_valid == 0u ||
        entry->direction != T425_CHANNEL_DMA_RX ||
        entry->endpoint != endpoint_id ||
        entry->physical_link != physical_link ||
        !t425_channel_compat_input_route_from_config(
            architectural_channel, endpoint_id, physical_link, lane_id, &route) ||
        route.stack_lane != entry->lane ||
        route.stack_lane != route52->dma_slot)
        return false;
    key.route = route;
    key.request_id = request_id;
    key.generation = operation_generation;
    bound = &adapter->endpoint[endpoint_id];
    if (bound->bound != 0u) {
        if (!t425_channel_compat_input_key_matches(
                &adapter->input->lane[route.input_lane], &key))
            return false;
        return fetch_active_request(adapter, bound, core, &request);
    }
    if ((bound->held_valid != 0u &&
         bound->held_frame > UINT32_C(0xFF)) ||
        !t425_core_channel_admission_get_active_compat_input(
            core, architectural_channel, request_id, operation_generation,
            endpoint_id, physical_link, &request) ||
        request.direction != T425_CHANNEL_DMA_RX ||
        request.lane != route.stack_lane ||
        request.architectural_channel != architectural_channel ||
        request.endpoint != endpoint_id || request.physical_link != physical_link)
        return false;

    if (entry->push_count != 2u || entry->enabled == 0u ||
        entry->descriptor_valid == 0u || entry->channel_word_published == 0u ||
        entry->owner_is_f05 != 0u ||
        entry->state != T425_CHANNEL_ADMISSION_21_ACTIVE)
        return false;
    f17 = &core->t425_channel_dma.rx[request.lane];
    if (f17->count_reg != request.initial_count_reg ||
        f17->ptr_reg != (request.buffer_address & ~UINT32_C(3)) ||
        f17->dbuff_reg != entry->initial_dbuff_reg ||
        !t425_channel_compat_input_capture_after_pushes_at_offset(
            adapter->input, &route, request.request_id,
            request.operation_generation, entry->push_count,
            f17->count_reg, f17->ptr_reg, f17->dbuff_reg,
            (uint8_t)(request.buffer_address & 3u), entry->enabled != 0u))
        return false;

    bound->key = key;
    bound->bound = 1u;
    return true;
}

bool t425_channel_compat_rx_adapter_07_retire_completed(
    t425_channel_compat_rx_adapter_07_t *adapter, t425_core_t *core,
    uint32_t endpoint_id)
{
    t425_channel_compat_rx_07_endpoint_t *endpoint;
    t425_channel_compat_input_lane_t *lane;
    const t425_channel_compat_input_key_t *key;
    const t425_channel_admission_entry_21_t *entry;
    const t425_channel_route_52_t *route52;
    t425_channel_admission_request_21_t active_request;
    uint32_t address;

    if (adapter == NULL || core == NULL || adapter->input == NULL ||
        endpoint_id >= T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS)
        return false;
    endpoint = &adapter->endpoint[endpoint_id];
    if (endpoint->bound == 0u)
        return false;
    key = &endpoint->key;
    address = key->route.architectural_channel;
    if (!t425_channel_compat_input_route_valid(&key->route) ||
        key->request_id == 0u || key->generation == 0u ||
        key->route.endpoint_id != endpoint_id ||
        key->route.lane_id != adapter->lane_id[key->route.input_lane])
        return false;

    entry = admission_find(core, address);
    route52 = route_52_find(address);
    if (entry == NULL || route52 == NULL ||
        route52->direction != T425_CHANNEL_DMA_RX ||
        route52->dma_slot != key->route.stack_lane ||
        route52->endpoint != endpoint_id ||
        route52->physical_link != key->route.physical_link ||
        entry->architectural_channel != address ||
        entry->operation_generation != key->generation ||
        entry->request_id != key->request_id ||
        entry->direction != T425_CHANNEL_DMA_RX ||
        entry->lane != key->route.stack_lane ||
        entry->endpoint != endpoint_id ||
        entry->physical_link != key->route.physical_link ||
        entry->route_valid == 0u || entry->enabled == 0u ||
        entry->owner_is_f05 != 0u ||
        (entry->state != T425_CHANNEL_ADMISSION_21_ACTIVE &&
         entry->state != T425_CHANNEL_ADMISSION_21_COMPLETED))
        return false;

    lane = &adapter->input->lane[key->route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, key) ||
        lane->active == 0u || lane->input_complete == 0u ||
        lane->input_state != T425_CHANNEL_COMPAT_INPUT_IDLE_14 ||
        lane->count_reg != 0u || lane->input_data_valid != 0u ||
        lane->input_ack != 0u || lane->ubus_request_pending != 0u ||
        lane->staging_mask != 0u || lane->input_byte_count != 0u ||
        lane->push_mailbox.valid != 0u ||
        !f17_matches_compat_lane(adapter, endpoint, core))
        return false;

    if (entry->state == T425_CHANNEL_ADMISSION_21_ACTIVE) {
        if (!t425_core_channel_admission_get_active_compat_input(
                core, address, key->request_id, key->generation,
                (uint8_t)endpoint_id, key->route.physical_link,
                &active_request) ||
            active_request.direction != T425_CHANNEL_DMA_RX ||
            active_request.lane != key->route.stack_lane ||
            active_request.endpoint != endpoint_id ||
            active_request.physical_link != key->route.physical_link ||
            !t425_core_channel_admission_report_complete_compat_input(
                core, address, key->generation))
            return false;
    }

    /* COMPLETED is intentionally accepted here for a retry of retire only. */
    if (!t425_core_channel_admission_retire_compat_input(
            core, address, key->generation))
        return false;
    if (!t425_channel_compat_input_retire_completed(adapter->input, key))
        return false;

    /* The pending transport byte belongs to the endpoint, not this session. */
    endpoint->bound = 0u;
    memset(&endpoint->key, 0, sizeof(endpoint->key));
    return true;
}

t425_channel_compat_rx_07_result_t
 t425_channel_compat_rx_adapter_07_poll_endpoint(
    t425_channel_compat_rx_adapter_07_t *adapter, const t425_core_t *core,
    uint32_t endpoint_id)
{
    t425_channel_compat_rx_07_endpoint_t *endpoint;
    t425_channel_admission_request_21_t request;
    t425_channel_compat_input_offer_result_t offered;

    if (adapter == NULL || adapter->phy == NULL || adapter->input == NULL ||
        endpoint_id >= T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS)
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    endpoint = &adapter->endpoint[endpoint_id];
    if (!fetch_active_request(adapter, endpoint, core, &request))
        return T425_CHANNEL_COMPAT_RX_07_STALE;
    if (adapter->input->lane[endpoint->key.route.input_lane].input_complete != 0u)
        return T425_CHANNEL_COMPAT_RX_07_COMPLETE;

    if (endpoint->held_valid == 0u) {
        if (!t425_core_phy_rx_available(adapter->phy, endpoint_id))
            return T425_CHANNEL_COMPAT_RX_07_NO_DATA;
        if (t425_core_phy_rx(adapter->phy, endpoint_id,
                             &endpoint->held_frame) != 0)
            return T425_CHANNEL_COMPAT_RX_07_INVALID;
        endpoint->held_valid = 1u;
    }
    if (endpoint->held_frame > UINT32_C(0xFF))
        return T425_CHANNEL_COMPAT_RX_07_INVALID;

    offered = t425_channel_compat_input_offer_rx(
        adapter->input, &endpoint->key, (uint8_t)endpoint->held_frame);
    if (offered == T425_CHANNEL_COMPAT_INPUT_OFFER_ACCEPTED) {
        endpoint->held_frame = 0u;
        endpoint->held_valid = 0u;
        return T425_CHANNEL_COMPAT_RX_07_ACCEPTED;
    }
    if (offered == T425_CHANNEL_COMPAT_INPUT_OFFER_BACKPRESSURE)
        return T425_CHANNEL_COMPAT_RX_07_RETAINED;
    if (offered == T425_CHANNEL_COMPAT_INPUT_OFFER_STALE_SESSION)
        return T425_CHANNEL_COMPAT_RX_07_STALE;
    return T425_CHANNEL_COMPAT_RX_07_INVALID;
}

t425_channel_compat_rx_07_result_t
 t425_channel_compat_rx_adapter_07_step_endpoint(
    t425_channel_compat_rx_adapter_07_t *adapter, t425_core_t *core,
    uint32_t endpoint_id)
{
    t425_channel_compat_rx_07_endpoint_t *endpoint;
    t425_channel_compat_input_lane_t *lane;
    t425_channel_admission_request_21_t request;
    t425_channel_compat_input_step_result_t transition;

    if (adapter == NULL || adapter->input == NULL || adapter->f04 == NULL ||
        endpoint_id >= T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS)
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    endpoint = &adapter->endpoint[endpoint_id];
    if (!fetch_active_request(adapter, endpoint, core, &request))
        return T425_CHANNEL_COMPAT_RX_07_STALE;
    if (adapter->f04 != &core->channel_dma_memory ||
        adapter->f04->bank != &core->t425_channel_dma ||
        adapter->f04->memory != &core->memory)
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    lane = &adapter->input->lane[endpoint->key.route.input_lane];
    if (lane->input_complete != 0u)
        return T425_CHANNEL_COMPAT_RX_07_COMPLETE;

    if (lane->input_data_valid != 0u) {
        transition = t425_channel_compat_input_step(
            adapter->input, endpoint->key.route.input_lane);
        if (transition != T425_CHANNEL_COMPAT_INPUT_STEP_DATA_CONSUMED)
            return T425_CHANNEL_COMPAT_RX_07_INVALID;
        publish_f17_compat_lane(adapter, endpoint, core);
        return T425_CHANNEL_COMPAT_RX_07_DATA_CONSUMED;
    }
    if (lane->input_ack != 0u) {
        if (!t425_channel_compat_input_acknowledge(
                adapter->input, &endpoint->key))
            return T425_CHANNEL_COMPAT_RX_07_INVALID;
        return T425_CHANNEL_COMPAT_RX_07_ACK_CLEARED;
    }
    if (lane->input_state == T425_CHANNEL_COMPAT_INPUT_POST_WRITE_1A) {
        transition = t425_channel_compat_input_step(
            adapter->input, endpoint->key.route.input_lane);
        if (transition == T425_CHANNEL_COMPAT_INPUT_STEP_COMPLETE)
            return T425_CHANNEL_COMPAT_RX_07_COMPLETE;
        if (transition != T425_CHANNEL_COMPAT_INPUT_STEP_POST_WRITE)
            return T425_CHANNEL_COMPAT_RX_07_INVALID;
        publish_f17_compat_lane(adapter, endpoint, core);
        return T425_CHANNEL_COMPAT_RX_07_POST_WRITE;
    }

    if (lane->input_state == T425_CHANNEL_COMPAT_INPUT_FULL_WORD_18 ||
        lane->input_state == T425_CHANNEL_COMPAT_INPUT_PARTIAL_19 ||
        lane->input_state == T425_CHANNEL_COMPAT_INPUT_UNALIGNED_COMMIT_1C) {
        t425_channel_compat_input_commit_receipt_t receipt;
        t425_channel_dma_ubus_transfer_status_t status;
        t425_channel_compat_input_apply_result_t applied;
        if (lane->ubus_request_pending == 0u &&
            !t425_channel_compat_input_prepare_next_commit(
                adapter->input, &endpoint->key))
            return T425_CHANNEL_COMPAT_RX_07_INVALID;
        status = t425_channel_dma_memory_04_rx_commit_compat(
            adapter->f04, &endpoint->key, lane->ubus_request_id,
            lane->ubus_request_address, lane->ubus_request_data,
            lane->ubus_byte_mask, &receipt);
        applied = t425_channel_compat_input_apply_receipt(
            adapter->input, &receipt);
        if (status == T425_CHANNEL_DMA_UBUS_TRANSFER_RETRY &&
            applied == T425_CHANNEL_COMPAT_INPUT_COMMIT_RETRY_PENDING)
            return T425_CHANNEL_COMPAT_RX_07_COMMIT_RETRY;
        if (status == T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE &&
            applied == T425_CHANNEL_COMPAT_INPUT_COMMIT_APPLIED) {
            publish_f17_compat_lane(adapter, endpoint, core);
            return T425_CHANNEL_COMPAT_RX_07_COMMIT_SUCCESS;
        }
        if (applied == T425_CHANNEL_COMPAT_INPUT_COMMIT_ERROR)
            return T425_CHANNEL_COMPAT_RX_07_COMMIT_ERROR;
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    }
    return T425_CHANNEL_COMPAT_RX_07_NO_ACTION;
}

bool t425_channel_rx_dispatch_07_init(
    t425_channel_rx_dispatch_07_t *dispatch,
    t425_channel_rx_dispatch_07_mode_t mode, t425_core_t *core,
    t425_channel_dma_rx_adapter_10_t *legacy,
    t425_channel_compat_rx_adapter_07_t *compat)
{
    if (dispatch == NULL || core == NULL ||
        (mode != T425_CHANNEL_RX_DISPATCH_07_LEGACY_F05 &&
         mode != T425_CHANNEL_RX_DISPATCH_07_COMPAT_INPUT) ||
        legacy == NULL || compat == NULL)
        return false;
    for (uint32_t i = 0u; i < T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS; ++i) {
        if (mode == T425_CHANNEL_RX_DISPATCH_07_COMPAT_INPUT) {
            const t425_channel_dma_rx_adapter_10_endpoint_t *old =
                &legacy->endpoint[i];
            if (old->active || old->held_valid || old->drain_pending ||
                old->session_bound || compat->endpoint[i].bound ||
                compat->endpoint[i].held_valid || compat->input == NULL ||
                compat->input->lane[i].active ||
                compat->input->lane[i].input_data_valid ||
                compat->input->lane[i].input_ack ||
                compat->input->lane[i].ubus_request_pending)
                return false;
        } else if (compat->endpoint[i].bound || compat->endpoint[i].held_valid ||
                   compat->input == NULL || compat->input->lane[i].active ||
                   compat->input->lane[i].input_data_valid ||
                   compat->input->lane[i].input_ack ||
                   compat->input->lane[i].ubus_request_pending) {
            return false;
        }
    }
    dispatch->mode = mode;
    dispatch->core = core;
    dispatch->legacy = legacy;
    dispatch->compat = compat;
    return true;
}

t425_channel_compat_rx_07_result_t t425_channel_rx_dispatch_07_poll(
    t425_channel_rx_dispatch_07_t *dispatch, uint32_t endpoint)
{
    if (dispatch == NULL || endpoint >= T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS)
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    if (dispatch->mode == T425_CHANNEL_RX_DISPATCH_07_COMPAT_INPUT)
        return t425_channel_compat_rx_adapter_07_poll_endpoint(
            dispatch->compat, dispatch->core, endpoint);
    switch (t425_channel_dma_rx_adapter_10_poll_endpoint(
        dispatch->legacy, endpoint)) {
    case T425_CHANNEL_DMA_RX_ADAPTER_10_NO_DATA:
        return T425_CHANNEL_COMPAT_RX_07_NO_DATA;
    case T425_CHANNEL_DMA_RX_ADAPTER_10_RETAINED:
        return T425_CHANNEL_COMPAT_RX_07_RETAINED;
    case T425_CHANNEL_DMA_RX_ADAPTER_10_ACCEPTED:
    case T425_CHANNEL_DMA_RX_ADAPTER_10_ACCEPTED_RETRY:
    case T425_CHANNEL_DMA_RX_ADAPTER_10_DRAINED:
        return T425_CHANNEL_COMPAT_RX_07_ACCEPTED;
    case T425_CHANNEL_DMA_RX_ADAPTER_10_IDLE:
        return T425_CHANNEL_COMPAT_RX_07_NO_ACTION;
    default:
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    }
}

t425_channel_compat_rx_07_result_t t425_channel_rx_dispatch_07_step(
    t425_channel_rx_dispatch_07_t *dispatch, uint32_t endpoint)
{
    if (dispatch == NULL ||
        dispatch->mode != T425_CHANNEL_RX_DISPATCH_07_COMPAT_INPUT)
        return T425_CHANNEL_COMPAT_RX_07_INVALID;
    return t425_channel_compat_rx_adapter_07_step_endpoint(
        dispatch->compat, dispatch->core, endpoint);
}


static void core_profile_microstep_08(void *context, void *core_ptr)
{
    t425_channel_compat_rx_adapter_07_t *adapter =
        (t425_channel_compat_rx_adapter_07_t *)context;
    t425_core_t *core = (t425_core_t *)core_ptr;
    if (adapter == NULL || core == NULL)
        return;
    for (uint32_t endpoint = 0u;
         endpoint < T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS; ++endpoint) {
        t425_channel_compat_rx_07_endpoint_t *bound =
            &adapter->endpoint[endpoint];
        t425_channel_compat_input_lane_t *lane;
        t425_channel_compat_rx_07_result_t result;

        if (bound->bound == 0u)
            continue;
        lane = &adapter->input->lane[bound->key.route.input_lane];
        if (lane->input_complete != 0u) {
            (void)t425_channel_compat_rx_adapter_07_retire_completed(
                adapter, core, endpoint);
            continue;
        }
        result = t425_channel_compat_rx_adapter_07_step_endpoint(
            adapter, core, endpoint);
        if (result == T425_CHANNEL_COMPAT_RX_07_COMPLETE)
            (void)t425_channel_compat_rx_adapter_07_retire_completed(
                adapter, core, endpoint);
    }
}

static uint8_t core_profile_reduce_08(void *context)
{
    const t425_channel_compat_rx_adapter_07_t *adapter =
        (const t425_channel_compat_rx_adapter_07_t *)context;
    return adapter == NULL ? 0u :
        t425_channel_compat_input_reduce_requests(adapter->input);
}

static bool core_profile_clear_08(
    void *context, uint32_t selected_channel, uint16_t mir_control)
{
    t425_channel_compat_rx_adapter_07_t *adapter =
        (t425_channel_compat_rx_adapter_07_t *)context;
    return adapter != NULL &&
        t425_channel_compat_input_clear_selected(
            adapter->input, selected_channel, mir_control);
}

static bool core_profile_service_08(void *context, void *core_ptr)
{
    t425_channel_compat_rx_adapter_07_t *adapter =
        (t425_channel_compat_rx_adapter_07_t *)context;
    t425_core_t *core = (t425_core_t *)core_ptr;
    static const uint32_t addresses[4] = {
        0x80000010u, 0x80000014u, 0x80000018u, 0x8000001Cu
    };
    bool progressed = false;
    if (adapter == NULL || core == NULL)
        return false;
    for (uint8_t i = 0u; i < 4u; ++i) {
        const t425_channel_admission_entry_21_t *entry =
            &core->channel_admission.channel[i];
        if (entry->state != T425_CHANNEL_ADMISSION_21_ACTIVE ||
            entry->owner_is_f05 != 0u)
            continue;
        if (!t425_channel_compat_rx_adapter_07_adopt_active(
                adapter, core, addresses[i], entry->request_id,
                entry->operation_generation, entry->endpoint,
                entry->physical_link, adapter->lane_id[i]))
            continue;
        {
            t425_channel_compat_rx_07_result_t result =
                t425_channel_compat_rx_adapter_07_poll_endpoint(
                    adapter, core, entry->endpoint);
            if (result == T425_CHANNEL_COMPAT_RX_07_ACCEPTED ||
                result == T425_CHANNEL_COMPAT_RX_07_RETAINED)
                progressed = true;
        }
    }
    return progressed;
}

bool t425_core_channel_compat_profile_08_enable(
    t425_core_t *core, t425_channel_compat_rx_adapter_07_t *adapter,
    const uint8_t lane_ids[4])
{
    if (core == NULL || adapter == NULL || lane_ids == NULL ||
        core->phy == NULL || !core->phy->attached ||
        adapter->phy != core->phy || adapter->input == NULL ||
        adapter->f04 != &core->channel_dma_memory ||
        core->channel_dma_rx_adapter_10_initialized == 0u)
        return false;
    for (uint8_t i = 0u; i < 4u; ++i) {
        const t425_channel_dma_rx_adapter_10_endpoint_t *legacy =
            &core->channel_dma_rx_adapter_10.endpoint[i];
        if (legacy->active || legacy->held_valid || legacy->drain_pending ||
            legacy->session_bound || core->channel_dma_shift.rx[i].configured ||
            core->channel_dma_shift.rx[i].active || adapter->endpoint[i].bound ||
            adapter->endpoint[i].held_valid ||
            adapter->input->lane[i].active)
            return false;
        for (uint8_t j = 0u; j < i; ++j)
            if (lane_ids[i] == lane_ids[j])
                return false;
    }
    if (!t425_channel_admission_21_configure_rx_profile(
            &core->channel_admission,
            T425_CHANNEL_RX_PROFILE_COMPAT_INPUT_FROZEN))
        return false;
    memcpy(adapter->lane_id, lane_ids, sizeof(adapter->lane_id));
    core->channel_compat_profile_08_context = adapter;
    core->channel_compat_profile_08_microstep = core_profile_microstep_08;
    core->channel_compat_profile_08_reduce = core_profile_reduce_08;
    core->channel_compat_profile_08_clear = core_profile_clear_08;
    core->channel_compat_profile_08_service = core_profile_service_08;
    return true;
}
