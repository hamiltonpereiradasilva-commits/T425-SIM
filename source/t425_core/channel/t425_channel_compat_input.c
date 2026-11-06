#include "t425_channel_compat_input.h"

#include <string.h>

static bool same_key(
    const t425_channel_compat_input_key_t *left,
    const t425_channel_compat_input_key_t *right);

bool t425_channel_compat_input_decode_address(
    uint32_t architectural_channel, uint8_t *input_lane,
    uint8_t *stack_lane, uint8_t *shared_index)
{
    uint32_t offset;
    uint8_t lane;

    if (input_lane == NULL || stack_lane == NULL || shared_index == NULL ||
        architectural_channel < 0x80000010u ||
        architectural_channel > 0x8000001Cu ||
        (architectural_channel & 3u) != 0u) {
        return false;
    }

    offset = architectural_channel - 0x80000010u;
    lane = (uint8_t)(offset / 4u);
    if (lane >= T425_CHANNEL_COMPAT_INPUT_LANES) {
        return false;
    }

    *input_lane = lane;
    *stack_lane = lane;
    *shared_index = (uint8_t)(lane + T425_CHANNEL_COMPAT_INPUT_LANES);
    return true;
}

bool t425_channel_compat_input_route_from_config(
    uint32_t architectural_channel, uint8_t endpoint_id,
    uint8_t physical_link, uint8_t lane_id,
    t425_channel_compat_input_route_t *route)
{
    uint8_t input_lane, stack_lane, shared_index;

    if (route == NULL ||
        !t425_channel_compat_input_decode_address(
            architectural_channel, &input_lane, &stack_lane, &shared_index)) {
        return false;
    }
    route->architectural_channel = architectural_channel;
    route->input_lane = input_lane;
    route->stack_lane = stack_lane;
    route->shared_index = shared_index;
    route->endpoint_id = endpoint_id;
    route->physical_link = physical_link;
    route->lane_id = lane_id;
    return true;
}

bool t425_channel_compat_input_route_valid(
    const t425_channel_compat_input_route_t *route)
{
    uint8_t input_lane, stack_lane, shared_index;

    if (route == NULL ||
        !t425_channel_compat_input_decode_address(
            route->architectural_channel, &input_lane,
            &stack_lane, &shared_index)) {
        return false;
    }

    return route->input_lane == input_lane &&
           route->stack_lane == stack_lane &&
           route->shared_index == shared_index;
}

void t425_channel_compat_input_init(t425_channel_compat_input_t *input)
{
    if (input != NULL) {
        memset(input, 0, sizeof(*input));
        for (uint8_t i = 0u; i < T425_CHANNEL_COMPAT_INPUT_LANES; ++i) {
            input->lane[i].input_state = T425_CHANNEL_COMPAT_INPUT_IDLE_14;
        }
    }
}

bool t425_channel_compat_input_capture_after_pushes_at_offset(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_route_t *route,
    uint32_t request_id, uint32_t generation,
    uint8_t completed_push_count,
    uint32_t count_reg, uint32_t ptr_reg, uint32_t dbuff_reg,
    uint8_t input_byte_offset, bool enable)
{
    t425_channel_compat_input_lane_t *lane;
    uint8_t persistent_run_req;
    uint8_t persistent_pri_out;
    uint32_t persistent_next_ubus_request_id;
    t425_channel_compat_input_push_mailbox_t mailbox;
    uint8_t persistent_owner_valid;
    t425_channel_compat_input_key_t persistent_owner;

    if (input == NULL || !t425_channel_compat_input_route_valid(route) ||
        request_id == 0u || generation == 0u || completed_push_count != 2u ||
        (count_reg & 0x80000000u) == 0u || count_reg == 0u ||
        input_byte_offset > 3u) {
        return false;
    }

    lane = &input->lane[route->input_lane];
    if (lane->active || lane->push_mailbox.valid != 0u) {
        return false;
    }

    persistent_run_req = lane->input_run_req;
    persistent_pri_out = lane->input_pri_out;
    persistent_next_ubus_request_id = lane->next_ubus_request_id;
    mailbox = lane->push_mailbox;
    persistent_owner_valid = lane->run_req_owner_valid;
    persistent_owner = lane->run_req_owner;
    memset(lane, 0, sizeof(*lane));
    lane->input_run_req = persistent_run_req;
    lane->input_pri_out = persistent_pri_out;
    lane->next_ubus_request_id = persistent_next_ubus_request_id != 0u ?
        persistent_next_ubus_request_id : request_id;
    lane->push_mailbox = mailbox;
    lane->run_req_owner_valid = persistent_owner_valid;
    lane->run_req_owner = persistent_owner;
    lane->active = 1u;
    lane->route = *route;
    lane->request_id = request_id;
    lane->generation = generation;
    lane->initial_count_reg = count_reg;
    lane->initial_ptr_reg = ptr_reg;
    lane->initial_dbuff_reg = dbuff_reg;
    lane->count_reg = count_reg;
    lane->ptr_reg = ptr_reg;
    lane->dbuff_reg = dbuff_reg;
    lane->input_num_bytes_to_transfer = 0u - count_reg;
    lane->input_byte_offset = input_byte_offset;
    lane->input_state = lane->input_byte_offset != 0u ?
        T425_CHANNEL_COMPAT_INPUT_UNALIGNED_1B :
        (lane->input_num_bytes_to_transfer < 4u ?
         T425_CHANNEL_COMPAT_INPUT_RECEIVE_17 :
         T425_CHANNEL_COMPAT_INPUT_RECEIVE_16);
    /* F21 ACTIVE proves Enable was asserted; seed it once, never clear it
     * while replacing an operation context. Only the MIR clear path owns reset. */
    if (enable)
        input->subsystem_enable_latch[route->shared_index] = 1u;
    return true;
}

bool t425_channel_compat_input_capture_after_pushes(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_route_t *route,
    uint32_t request_id, uint32_t generation,
    uint8_t completed_push_count,
    uint32_t count_reg, uint32_t ptr_reg, uint32_t dbuff_reg,
    bool enable)
{
    return t425_channel_compat_input_capture_after_pushes_at_offset(
        input, route, request_id, generation, completed_push_count,
        count_reg, ptr_reg, dbuff_reg, (uint8_t)(ptr_reg & 3u), enable);
}

bool t425_channel_compat_input_key_matches(
    const t425_channel_compat_input_lane_t *lane,
    const t425_channel_compat_input_key_t *key)
{
    return lane != NULL && key != NULL && lane->active != 0u &&
           key->request_id == lane->request_id &&
           key->generation == lane->generation &&
           key->route.architectural_channel == lane->route.architectural_channel &&
           key->route.input_lane == lane->route.input_lane &&
           key->route.stack_lane == lane->route.stack_lane &&
           key->route.shared_index == lane->route.shared_index &&
           key->route.endpoint_id == lane->route.endpoint_id &&
           key->route.physical_link == lane->route.physical_link &&
           key->route.lane_id == lane->route.lane_id;
}

t425_channel_compat_input_offer_result_t
t425_channel_compat_input_offer_rx(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key, uint8_t byte)
{
    t425_channel_compat_input_lane_t *lane;

    if (input == NULL || key == NULL ||
        !t425_channel_compat_input_route_valid(&key->route)) {
        return T425_CHANNEL_COMPAT_INPUT_OFFER_INVALID;
    }
    lane = &input->lane[key->route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, key)) {
        return T425_CHANNEL_COMPAT_INPUT_OFFER_STALE_SESSION;
    }
    if (lane->input_data_valid != 0u || lane->input_ack != 0u ||
        lane->input_complete != 0u || lane->ubus_request_pending != 0u ||
        (lane->input_state != T425_CHANNEL_COMPAT_INPUT_RECEIVE_16 &&
         lane->input_state != T425_CHANNEL_COMPAT_INPUT_RECEIVE_17 &&
         lane->input_state != T425_CHANNEL_COMPAT_INPUT_UNALIGNED_1B) ||
        ((uint32_t)lane->input_byte_offset + lane->input_byte_count) >= 4u) {
        return T425_CHANNEL_COMPAT_INPUT_OFFER_BACKPRESSURE;
    }

    lane->input_data = byte;
    lane->input_data_valid = 1u;
    return T425_CHANNEL_COMPAT_INPUT_OFFER_ACCEPTED;
}

bool t425_channel_compat_input_consume_data(
    t425_channel_compat_input_t *input, uint8_t input_lane)
{
    t425_channel_compat_input_lane_t *lane;
    uint32_t shift;
    uint32_t byte_count;

    if (input == NULL || input_lane >= T425_CHANNEL_COMPAT_INPUT_LANES) {
        return false;
    }
    lane = &input->lane[input_lane];
    if (lane->active == 0u || lane->input_data_valid == 0u ||
        lane->input_ack != 0u ||
        (lane->input_state != T425_CHANNEL_COMPAT_INPUT_RECEIVE_16 &&
         lane->input_state != T425_CHANNEL_COMPAT_INPUT_RECEIVE_17 &&
         lane->input_state != T425_CHANNEL_COMPAT_INPUT_UNALIGNED_1B) ||
        lane->count_reg == 0u) {
        return false;
    }

    byte_count = lane->input_byte_count;
    shift = 8u * ((uint32_t)lane->input_byte_offset + byte_count);
    if (shift >= 32u) {
        return false;
    }

    lane->dbuff_reg = (lane->dbuff_reg & ~(UINT32_C(0xFF) << shift)) |
        ((uint32_t)lane->input_data << shift);
    lane->staging_mask |= (uint8_t)(1u << (shift / 8u));
    lane->input_byte_count++;
    lane->input_data_valid = 0u;
    lane->input_ack = 1u;
    return true;
}

bool t425_channel_compat_input_acknowledge(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key)
{
    t425_channel_compat_input_lane_t *lane;

    if (input == NULL || key == NULL ||
        !t425_channel_compat_input_route_valid(&key->route)) {
        return false;
    }
    lane = &input->lane[key->route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, key) ||
        lane->input_ack == 0u || lane->input_data_valid != 0u) {
        return false;
    }
    lane->input_ack = 0u;
    {
        uint32_t remaining = 0u - lane->count_reg;
        uint32_t occupied = (uint32_t)lane->input_byte_offset +
            lane->input_byte_count;
        if (occupied == 4u) {
            lane->input_state = lane->input_byte_offset == 0u ?
                T425_CHANNEL_COMPAT_INPUT_FULL_WORD_18 :
                T425_CHANNEL_COMPAT_INPUT_UNALIGNED_COMMIT_1C;
        } else if (lane->input_byte_count >= remaining) {
            lane->input_state = lane->input_byte_offset == 0u ?
                T425_CHANNEL_COMPAT_INPUT_PARTIAL_19 :
                T425_CHANNEL_COMPAT_INPUT_UNALIGNED_COMMIT_1C;
        }
    }
    return true;
}

t425_channel_compat_input_step_result_t t425_channel_compat_input_step(
    t425_channel_compat_input_t *input, uint8_t input_lane)
{
    t425_channel_compat_input_lane_t *lane;
    uint32_t remaining;

    if (input == NULL || input_lane >= T425_CHANNEL_COMPAT_INPUT_LANES)
        return T425_CHANNEL_COMPAT_INPUT_STEP_INVALID;
    lane = &input->lane[input_lane];
    if (lane->active == 0u)
        return T425_CHANNEL_COMPAT_INPUT_STEP_INVALID;
    if (lane->input_data_valid != 0u) {
        if (!t425_channel_compat_input_consume_data(input, input_lane))
            return T425_CHANNEL_COMPAT_INPUT_STEP_INVALID;
        return T425_CHANNEL_COMPAT_INPUT_STEP_DATA_CONSUMED;
    }
    if (lane->input_ack != 0u || lane->ubus_request_pending != 0u)
        return T425_CHANNEL_COMPAT_INPUT_STEP_NO_ACTION;
    if (lane->input_state != T425_CHANNEL_COMPAT_INPUT_POST_WRITE_1A)
        return T425_CHANNEL_COMPAT_INPUT_STEP_NO_ACTION;

    if (lane->count_reg == 0u) {
        if (lane->input_data_valid != 0u || lane->input_ack != 0u ||
            lane->ubus_request_pending != 0u || lane->staging_mask != 0u ||
            lane->input_byte_count != 0u) {
            return T425_CHANNEL_COMPAT_INPUT_STEP_INVALID;
        }
        if (!t425_channel_compat_input_route_valid(&lane->route) ||
            lane->request_id == 0u || lane->generation == 0u)
            return T425_CHANNEL_COMPAT_INPUT_STEP_INVALID;
        lane->run_req_owner.route = lane->route;
        lane->run_req_owner.request_id = lane->request_id;
        lane->run_req_owner.generation = lane->generation;
        lane->run_req_owner_valid = 1u;
        lane->input_num_bytes_to_transfer = 0u;
        lane->input_complete = 1u;
        lane->input_run_req = 1u;
        lane->input_state = T425_CHANNEL_COMPAT_INPUT_IDLE_14;
        return T425_CHANNEL_COMPAT_INPUT_STEP_COMPLETE;
    }
    remaining = 0u - lane->count_reg;
    lane->dbuff_reg = 0u;
    lane->input_byte_count = 0u;
    lane->input_byte_offset = (uint8_t)(lane->ptr_reg & 3u);
    lane->staging_mask = 0u;
    lane->input_num_bytes_to_transfer = remaining;
    lane->input_state = lane->input_byte_offset != 0u ?
        T425_CHANNEL_COMPAT_INPUT_UNALIGNED_1B :
        (remaining < 4u ? T425_CHANNEL_COMPAT_INPUT_RECEIVE_17 :
         T425_CHANNEL_COMPAT_INPUT_RECEIVE_16);
    return T425_CHANNEL_COMPAT_INPUT_STEP_POST_WRITE;
}

t425_channel_compat_input_push_result_t
 t425_channel_compat_input_stage_push(
    t425_channel_compat_input_t *input, uint8_t input_lane,
    t425_channel_compat_input_push_kind_t kind, uint32_t value)
{
    t425_channel_compat_input_lane_t *lane;

    if (input == NULL || input_lane >= T425_CHANNEL_COMPAT_INPUT_LANES ||
        (kind != T425_CHANNEL_COMPAT_PUSH_ZBUS &&
         kind != T425_CHANNEL_COMPAT_PUSH_TESTHARDCHAN)) {
        return T425_CHANNEL_COMPAT_PUSH_INVALID;
    }
    lane = &input->lane[input_lane];
    if (lane->active == 0u) {
        return T425_CHANNEL_COMPAT_PUSH_PASS_THROUGH;
    }
    if (lane->push_mailbox.valid != 0u) {
        return T425_CHANNEL_COMPAT_PUSH_BUSY;
    }
    lane->push_mailbox.valid = 1u;
    lane->push_mailbox.kind = kind;
    lane->push_mailbox.value = value;
    return T425_CHANNEL_COMPAT_PUSH_STORED;
}

bool t425_channel_compat_input_release_push_mailbox(
    t425_channel_compat_input_t *input, uint8_t input_lane,
    t425_channel_compat_input_push_mailbox_t *event)
{
    t425_channel_compat_input_lane_t *lane;

    if (input == NULL || event == NULL ||
        input_lane >= T425_CHANNEL_COMPAT_INPUT_LANES) {
        return false;
    }
    lane = &input->lane[input_lane];
    if (lane->active != 0u || lane->push_mailbox.valid == 0u) {
        return false;
    }
    *event = lane->push_mailbox;
    memset(&lane->push_mailbox, 0, sizeof(lane->push_mailbox));
    return true;
}

bool t425_channel_compat_input_retire(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key)
{
    t425_channel_compat_input_lane_t *lane;
    uint8_t persistent_run_req;
    uint8_t persistent_pri_out;
    uint32_t persistent_next_ubus_request_id;
    t425_channel_compat_input_push_mailbox_t mailbox;
    uint8_t persistent_owner_valid;
    t425_channel_compat_input_key_t persistent_owner;

    if (input == NULL || key == NULL ||
        !t425_channel_compat_input_route_valid(&key->route)) {
        return false;
    }
    lane = &input->lane[key->route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, key)) {
        return false;
    }
    persistent_run_req = lane->input_run_req;
    persistent_pri_out = lane->input_pri_out;
    persistent_next_ubus_request_id = lane->next_ubus_request_id;
    mailbox = lane->push_mailbox;
    persistent_owner_valid = lane->run_req_owner_valid;
    persistent_owner = lane->run_req_owner;
    memset(lane, 0, sizeof(*lane));
    lane->input_run_req = persistent_run_req;
    lane->input_pri_out = persistent_pri_out;
    lane->next_ubus_request_id = persistent_next_ubus_request_id;
    lane->push_mailbox = mailbox;
    lane->run_req_owner_valid = persistent_owner_valid;
    lane->run_req_owner = persistent_owner;
    lane->input_state = T425_CHANNEL_COMPAT_INPUT_IDLE_14;
    return true;
}

bool t425_channel_compat_input_retire_completed(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key)
{
    t425_channel_compat_input_lane_t *lane;

    if (input == NULL || key == NULL ||
        !t425_channel_compat_input_route_valid(&key->route)) {
        return false;
    }
    lane = &input->lane[key->route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, key) ||
        lane->input_complete == 0u ||
        lane->input_state != T425_CHANNEL_COMPAT_INPUT_IDLE_14 ||
        lane->count_reg != 0u || lane->input_data_valid != 0u ||
        lane->input_ack != 0u || lane->ubus_request_pending != 0u ||
        lane->staging_mask != 0u || lane->input_byte_count != 0u ||
        lane->push_mailbox.valid != 0u) {
        return false;
    }
    return t425_channel_compat_input_retire(input, key);
}

uint8_t t425_channel_compat_input_reduce_requests(
    const t425_channel_compat_input_t *input)
{
    if (input == NULL)
        return 0u;
    for (uint8_t i = 0u; i < T425_CHANNEL_COMPAT_INPUT_LANES; ++i) {
        uint8_t shared = (uint8_t)(i + T425_CHANNEL_COMPAT_INPUT_LANES);
        if (input->lane[i].input_run_req != 0u &&
            input->subsystem_enable_latch[shared] != 0u)
            return 1u;
    }
    return 0u;
}

t425_channel_ifnext_owner_result_t
t425_channel_compat_input_preview_unique_ifnext_owner(
    const t425_channel_compat_input_t *input, bool if_next,
    t425_channel_compat_input_key_t *candidate_owner)
{
    uint8_t found = 0u;
    t425_channel_compat_input_key_t candidate = {0};

    if (input == NULL || candidate_owner == NULL)
        return T425_CHANNEL_IFNEXT_OWNER_NONE;
    memset(candidate_owner, 0, sizeof(*candidate_owner));
    if (!if_next)
        return T425_CHANNEL_IFNEXT_OWNER_NONE;

    for (uint8_t i = 0u; i < T425_CHANNEL_COMPAT_INPUT_LANES; ++i) {
        const t425_channel_compat_input_lane_t *lane = &input->lane[i];
        uint8_t shared = (uint8_t)(i + T425_CHANNEL_COMPAT_INPUT_LANES);
        if (lane->input_run_req == 0u ||
            input->subsystem_enable_latch[shared] == 0u ||
            lane->input_state != T425_CHANNEL_COMPAT_INPUT_IDLE_14 ||
            lane->run_req_owner_valid == 0u ||
            !t425_channel_compat_input_route_valid(&lane->run_req_owner.route) ||
            lane->run_req_owner.route.input_lane != i ||
            lane->run_req_owner.request_id == 0u ||
            lane->run_req_owner.generation == 0u)
            continue;
        if (found != 0u)
            return T425_CHANNEL_IFNEXT_OWNER_AMBIGUOUS;
        candidate = lane->run_req_owner;
        found = 1u;
    }
    if (found == 0u)
        return T425_CHANNEL_IFNEXT_OWNER_NONE;
    *candidate_owner = candidate;
    return T425_CHANNEL_IFNEXT_OWNER_SELECTED;
}

t425_channel_ifnext_owner_result_t
t425_channel_compat_input_bind_unique_ifnext_owner(
    t425_channel_compat_input_t *input, bool if_next,
    t425_channel_compat_input_key_t *selected_owner)
{
    t425_channel_compat_input_key_t candidate = {0};
    t425_channel_ifnext_owner_result_t result;

    if (input == NULL || selected_owner == NULL)
        return T425_CHANNEL_IFNEXT_OWNER_NONE;
    memset(selected_owner, 0, sizeof(*selected_owner));
    if (!if_next)
        return T425_CHANNEL_IFNEXT_OWNER_NONE;
    result = t425_channel_compat_input_preview_unique_ifnext_owner(
        input, true, &candidate);
    if (result != T425_CHANNEL_IFNEXT_OWNER_SELECTED)
        return result;

    if (input->selected_owner_valid != 0u) {
        if (!same_key(&input->selected_owner, &candidate))
            return T425_CHANNEL_IFNEXT_OWNER_PENDING;
        *selected_owner = input->selected_owner;
        return T425_CHANNEL_IFNEXT_OWNER_SELECTED;
    }

    input->selected_owner = candidate;
    input->selected_owner_valid = 1u;
    *selected_owner = candidate;
    return T425_CHANNEL_IFNEXT_OWNER_SELECTED;
}

bool t425_channel_compat_input_clear_selected_owned(
    t425_channel_compat_input_t *input, uint32_t architectural_channel,
    const t425_channel_compat_input_key_t *selected_owner,
    uint16_t mir_channel_control)
{
    uint8_t input_lane, stack_lane, shared_index;
    t425_channel_compat_input_lane_t *lane;
    bool match;

    if (input == NULL || selected_owner == NULL ||
        (mir_channel_control & 0x002u) == 0u ||
        !t425_channel_compat_input_decode_address(
            architectural_channel, &input_lane, &stack_lane, &shared_index))
        return false;
    (void)stack_lane;
    (void)shared_index;
    if (input->selected_owner_valid == 0u ||
        !same_key(&input->selected_owner, selected_owner) ||
        selected_owner->route.architectural_channel != architectural_channel ||
        selected_owner->route.input_lane != input_lane)
        return false;

    lane = &input->lane[input_lane];
    match = lane->input_state == T425_CHANNEL_COMPAT_INPUT_IDLE_14 &&
        lane->input_run_req != 0u && lane->run_req_owner_valid != 0u &&
        same_key(&lane->run_req_owner, selected_owner);
    /* Any MIR-0x002 attempt consumes this IF Next token. A stale token cannot
     * be replayed later against a different latch generation. */
    input->selected_owner_valid = 0u;
    memset(&input->selected_owner, 0, sizeof(input->selected_owner));
    if (!match)
        return false;
    lane->input_run_req = 0u;
    lane->run_req_owner_valid = 0u;
    memset(&lane->run_req_owner, 0, sizeof(lane->run_req_owner));
    return true;
}

bool t425_channel_compat_input_clear_selected(
    t425_channel_compat_input_t *input, uint32_t architectural_channel,
    uint16_t mir_channel_control)
{
    if (input == NULL || input->selected_owner_valid == 0u)
        return false;
    return t425_channel_compat_input_clear_selected_owned(
        input, architectural_channel, &input->selected_owner,
        mir_channel_control);
}

bool t425_channel_compat_input_disable_selected(
    t425_channel_compat_input_t *input, uint32_t architectural_channel)
{
    uint8_t input_lane, stack_lane, shared_index;
    t425_channel_compat_input_lane_t *lane;

    if (input == NULL ||
        !t425_channel_compat_input_decode_address(
            architectural_channel, &input_lane, &stack_lane, &shared_index))
        return false;

    lane = &input->lane[input_lane];
    if (lane->active > 1u ||
        (lane->active != 0u &&
         (!t425_channel_compat_input_route_valid(&lane->route) ||
          lane->route.architectural_channel != architectural_channel ||
          lane->route.input_lane != input_lane ||
          lane->route.stack_lane != stack_lane ||
          lane->route.shared_index != shared_index)))
        return false;

    /* Eligibility is the request AND its directional enable. Disabling the
     * selected lane must not erase request, ACK, or transfer progress. */
    input->subsystem_enable_latch[shared_index] = 0u;
    return true;
}

static uint32_t expand_byte_mask(uint8_t mask)
{
    uint32_t expanded = 0u;
    for (uint8_t bit = 0u; bit < 4u; ++bit) {
        if ((mask & (uint8_t)(1u << bit)) != 0u) {
            expanded |= UINT32_C(0xFF) << (8u * bit);
        }
    }
    return expanded;
}

static uint8_t count_mask_bytes(uint8_t mask)
{
    uint8_t count = 0u;
    for (uint8_t bit = 0u; bit < 4u; ++bit) {
        if ((mask & (uint8_t)(1u << bit)) != 0u) {
            count++;
        }
    }
    return count;
}

static uint8_t contiguous_staging_mask_from_pointer(
    const t425_channel_compat_input_lane_t *lane)
{
    uint8_t mask = 0u;
    uint8_t offset = lane->input_byte_offset;

    for (uint8_t bit = offset; bit < 4u; ++bit) {
        if ((lane->staging_mask & (uint8_t)(1u << bit)) == 0u) {
            break;
        }
        mask |= (uint8_t)(1u << bit);
    }
    return mask;
}

static bool same_key(
    const t425_channel_compat_input_key_t *left,
    const t425_channel_compat_input_key_t *right)
{
    return left != NULL && right != NULL &&
        left->request_id == right->request_id &&
        left->generation == right->generation &&
        left->route.architectural_channel == right->route.architectural_channel &&
        left->route.input_lane == right->route.input_lane &&
        left->route.stack_lane == right->route.stack_lane &&
        left->route.shared_index == right->route.shared_index &&
        left->route.endpoint_id == right->route.endpoint_id &&
        left->route.physical_link == right->route.physical_link &&
        left->route.lane_id == right->route.lane_id;
}

bool t425_channel_compat_input_prepare_commit(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key,
    uint32_t ubus_request_id, uint32_t aligned_address,
    uint32_t write_data, uint8_t byte_mask)
{
    t425_channel_compat_input_lane_t *lane;
    uint8_t expected_mask;

    if (input == NULL || key == NULL || ubus_request_id == 0u ||
        (aligned_address & 3u) != 0u || byte_mask == 0u ||
        (byte_mask & 0xF0u) != 0u ||
        !t425_channel_compat_input_route_valid(&key->route)) {
        return false;
    }
    lane = &input->lane[key->route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, key) ||
        (lane->ptr_reg & ~UINT32_C(3)) != aligned_address ||
        lane->input_data_valid != 0u || lane->input_ack != 0u ||
        (lane->input_state != T425_CHANNEL_COMPAT_INPUT_FULL_WORD_18 &&
         lane->input_state != T425_CHANNEL_COMPAT_INPUT_PARTIAL_19 &&
         lane->input_state != T425_CHANNEL_COMPAT_INPUT_UNALIGNED_COMMIT_1C)) {
        return false;
    }
    expected_mask = contiguous_staging_mask_from_pointer(lane);
    if (byte_mask != expected_mask || expected_mask == 0u ||
        (lane->dbuff_reg & (expand_byte_mask(byte_mask))) !=
        (write_data & (expand_byte_mask(byte_mask)))) {
        return false;
    }
    if (lane->ubus_request_pending != 0u) {
        return lane->ubus_request_id == ubus_request_id &&
            lane->ubus_request_generation == lane->generation &&
            lane->ubus_request_address == aligned_address &&
            lane->ubus_request_data == write_data &&
            lane->ubus_byte_mask == byte_mask &&
            same_key(&lane->ubus_session, key);
    }

    lane->ubus_request_pending = 1u;
    lane->ubus_request_id = ubus_request_id;
    lane->ubus_request_generation = lane->generation;
    lane->ubus_request_address = aligned_address;
    lane->ubus_request_data = write_data;
    lane->ubus_byte_mask = byte_mask;
    lane->ubus_session = *key;
    return true;
}

bool t425_channel_compat_input_prepare_next_commit(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key)
{
    t425_channel_compat_input_lane_t *lane;
    uint8_t mask;
    uint32_t request_id;
    bool already_pending;
    if (input == NULL || key == NULL ||
        !t425_channel_compat_input_route_valid(&key->route))
        return false;
    lane = &input->lane[key->route.input_lane];
    mask = contiguous_staging_mask_from_pointer(lane);
    if (!t425_channel_compat_input_key_matches(lane, key) || mask == 0u)
        return false;
    already_pending = lane->ubus_request_pending != 0u;
    request_id = already_pending ? lane->ubus_request_id :
        lane->next_ubus_request_id;
    if (request_id == 0u ||
        !t425_channel_compat_input_prepare_commit(
            input, key, request_id, lane->ptr_reg & ~UINT32_C(3),
            lane->dbuff_reg, mask))
        return false;
    if (!already_pending) {
        lane->next_ubus_request_id = request_id + 1u;
        if (lane->next_ubus_request_id == 0u)
            lane->next_ubus_request_id = 1u;
    }
    return true;
}

t425_channel_compat_input_apply_result_t
t425_channel_compat_input_apply_receipt(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_commit_receipt_t *receipt)
{
    t425_channel_compat_input_lane_t *lane;
    uint8_t committed_bytes;
    uint32_t remaining, next_count, next_ptr;

    if (input == NULL || receipt == NULL ||
        !t425_channel_compat_input_route_valid(&receipt->session.route)) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_INVALID;
    }
    lane = &input->lane[receipt->session.route.input_lane];
    if (!t425_channel_compat_input_key_matches(lane, &receipt->session)) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_STALE;
    }
    if (lane->last_commit_valid != 0u &&
        lane->last_commit_request_id == receipt->ubus_request_id &&
        lane->last_commit_generation == receipt->session.generation &&
        lane->last_commit_address == receipt->aligned_address &&
        lane->last_commit_data == receipt->write_data &&
        lane->last_commit_mask == receipt->byte_mask) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_DUPLICATE;
    }
    if (lane->ubus_request_pending == 0u ||
        lane->ubus_request_id != receipt->ubus_request_id ||
        lane->ubus_request_generation != receipt->session.generation ||
        lane->ubus_request_address != receipt->aligned_address ||
        lane->ubus_request_data != receipt->write_data ||
        lane->ubus_byte_mask != receipt->byte_mask ||
        !same_key(&lane->ubus_session, &receipt->session)) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_STALE;
    }
    if (receipt->status == T425_CHANNEL_COMPAT_COMMIT_RETRY) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_RETRY_PENDING;
    }
    if (receipt->status == T425_CHANNEL_COMPAT_COMMIT_ERROR) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_ERROR;
    }
    if (receipt->status != T425_CHANNEL_COMPAT_COMMIT_SUCCESS ||
        (receipt->byte_mask & 0xF0u) != 0u || receipt->byte_mask == 0u ||
        (receipt->aligned_address & 3u) != 0u ||
        (lane->ptr_reg & ~UINT32_C(3)) != receipt->aligned_address ||
        contiguous_staging_mask_from_pointer(lane) != receipt->byte_mask ||
        (lane->dbuff_reg & (expand_byte_mask(receipt->byte_mask))) !=
        (receipt->write_data & (expand_byte_mask(receipt->byte_mask)))) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_INVALID;
    }

    committed_bytes = count_mask_bytes(receipt->byte_mask);
    remaining = 0u - lane->count_reg;
    if (committed_bytes == 0u || committed_bytes > remaining ||
        lane->ptr_reg > UINT32_MAX - 4u ||
        (lane->ptr_reg & 3u) != 0u) {
        return T425_CHANNEL_COMPAT_INPUT_COMMIT_INVALID;
    }
    next_count = lane->count_reg + committed_bytes;
    remaining = 0u - next_count;
    next_ptr = lane->ptr_reg + 4u;

    lane->count_reg = next_count;
    lane->ptr_reg = next_ptr;
    lane->input_num_bytes_to_transfer = 0u - next_count;
    lane->staging_mask &= (uint8_t)~receipt->byte_mask;
    if (next_count != 0u) {
        lane->dbuff_reg &= ~(expand_byte_mask(receipt->byte_mask));
    }
    lane->ubus_request_pending = 0u;
    lane->ubus_request_id = 0u;
    lane->ubus_request_generation = 0u;
    lane->ubus_request_address = 0u;
    lane->ubus_request_data = 0u;
    lane->ubus_byte_mask = 0u;
    memset(&lane->ubus_session, 0, sizeof(lane->ubus_session));
    lane->last_commit_valid = 1u;
    lane->last_commit_request_id = receipt->ubus_request_id;
    lane->last_commit_generation = receipt->session.generation;
    lane->last_commit_address = receipt->aligned_address;
    lane->last_commit_data = receipt->write_data;
    lane->last_commit_mask = receipt->byte_mask;
    lane->next_ubus_request_id = receipt->ubus_request_id + 1u;
    if (lane->next_ubus_request_id == 0u)
        lane->next_ubus_request_id = 1u;
    lane->input_state = T425_CHANNEL_COMPAT_INPUT_POST_WRITE_1A;

    if (lane->staging_mask == 0u) {
        lane->input_byte_count = 0u;
        if (next_count == 0u)
            lane->input_byte_offset = (uint8_t)(lane->ptr_reg & 3u);
    }

    return T425_CHANNEL_COMPAT_INPUT_COMMIT_APPLIED;
}
