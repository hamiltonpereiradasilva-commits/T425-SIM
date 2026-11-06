#include "t425_channel_vbc_model_req03_06.h"

static void observe(
    channel_vbc_store_observer_fn observer,
    void *context,
    channel_vbc_store_event_t event)
{
    if (observer != 0) {
        observer(context, event);
    }
}


bool channel_vbc_output_terminal_ack(
    channel_vbc_state_t *state, uint32_t slot, uint32_t count_post)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT || count_post != 0u)
        return false;
    state->output_run_req[slot] = 1u;
    return true;
}

bool channel_vbc_output_gate_set(channel_vbc_state_t *state, uint32_t slot)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT) return false;
    state->subsystem_enable_latch[slot] = 1u;
    return true;
}

bool channel_vbc_output_gate_clear(channel_vbc_state_t *state, uint32_t slot)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT) return false;
    state->subsystem_enable_latch[slot] = 0u;
    return true;
}

uint8_t channel_vbc_output_run_request_reduce(
    const channel_vbc_state_t *state)
{
    uint32_t i;
    if (state == 0) return 0u;
    for (i = 0u; i < CHANNEL_VBC_SLOT_COUNT; ++i)
        if (state->output_run_req[i] != 0u &&
            state->subsystem_enable_latch[i] != 0u) return 1u;
    return 0u;
}

bool channel_vbc_input_b_alt_slot0(channel_vbc_state_t *state)
{
    if (state == 0 ||
        state->input_channel_status[0] != CHANNEL_VBC_INPUT_B_ALT_STATUS ||
        state->input_data_valid[0] != 1u ||
        state->alt_enabled_latch[0] != 1u ||
        state->warnings != 0u) {
        return false;
    }

    /* VBC x86 path D4E2..D50D: status, counter, ready, enable, then RunReq. */
    state->input_channel_status[0] = CHANNEL_VBC_INPUT_ALT_READY_STATUS;
    state->count_reg[4] += 1u;
    state->input_ready_req[0] = 1u;
    state->subsystem_enable_latch[4] = 1u;
    state->input_run_req[0] = 1u;

    if ((state->link_pri_reg[4] & 1u) != 0u) {
        state->input_pri_out[0] = 0u;
    } else {
        state->input_pri_out[0] = 1u;
        state->input_channel_status[0] = CHANNEL_VBC_INPUT_B_ALT_STATUS;
        state->ptr_reg[4] = (state->ptr_reg[4] + 3u) & ~UINT32_C(3);
    }

    return true;
}

bool channel_vbc_clear_selected_input_slot0(channel_vbc_state_t *state)
{
    if (state == 0 ||
        state->mir_bytes[7] == 0u ||
        state->select_channel_reg != CHANNEL_VBC_INPUT_SLOT0_SELECT ||
        (state->input_channel_status[0] != CHANNEL_VBC_INPUT_B_ALT_STATUS &&
         state->input_channel_status[0] != CHANNEL_VBC_INPUT_ALT_READY_STATUS) ||
        state->mir_bytes[2] == 0u) {
        return false;
    }

    state->input_ready_req[0] = 0u;
    state->input_run_req[0] = 0u;
    state->alt_enabled_latch[0] = 0u;
    state->input_pri_out[0] = 0u;
    return true;
}

bool channel_vbc_publish_input_data_valid(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT ||
        state->input_data_valid[slot] != 0u ||
        state->link_in_serial_bus_usage_status[slot] != 1u ||
        state->waiting_for_input_ack[slot] != 0u ||
        (state->mir_bytes[9] != 0u && state->channel_selected[slot] == 1u)) {
        return false;
    }

    /* Static store order recovered at x86 0x10000C9F3..0x10000CA19. */
    state->input_data_valid[slot] = 1u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_DATA_VALID_SET);

    state->link_in_serial_bus_usage_status[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_LINK_IN_USAGE_CLEAR);

    state->waiting_for_input_ack[slot] = 1u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_WAITING_INPUT_ACK_SET);

    state->sending_input_ack[slot] = 1u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_SENDING_INPUT_ACK_SET);

    state->input_data[slot] = state->link_in_serial_bus_payload[slot];
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_DATA_COPY);

    return true;
}

bool channel_vbc_reset_selected_link_interface_slot(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT ||
        state->mir_bytes[9] == 0u || state->channel_selected[slot] != 1u) {
        return false;
    }

    /* _HandleLinkInterfaces reset block, x86 0x10000C9A0..0x10000C9C1+.
     * Effects are limited to the stores explicitly recovered in F05.
     */
    state->waiting_for_output_ack[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_WAITING_OUTPUT_ACK_CLEAR);

    state->output_data_valid[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_OUTPUT_DATA_VALID_CLEAR);

    state->link_out_serial_bus_usage_status[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_LINK_OUT_USAGE_CLEAR);

    state->sending_input_ack[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_SENDING_INPUT_ACK_CLEAR);

    state->waiting_for_input_ack[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_WAITING_INPUT_ACK_CLEAR);

    state->input_data_valid[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_DATA_VALID_CLEAR);

    state->link_in_serial_bus_usage_status[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_LINK_IN_USAGE_CLEAR_RESET);

    state->input_ack[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_ACK_CLEAR);

    return true;
}


static uint32_t input_shift_for_byte(uint32_t offset, uint32_t count)
{
    /* x86 SHL masks its 32-bit shift count to five bits. */
    return ((offset + count) * 8u) & 31u;
}

static channel_vbc_input_consume_result_t consume_status_17(
    channel_vbc_state_t *state, uint32_t slot,
    channel_vbc_store_observer_fn observer, void *observer_context)
{
    uint32_t old_count = state->input_byte_count[slot];
    uint32_t new_count = old_count + 1u;
    uint32_t shift = input_shift_for_byte(state->input_byte_offset[slot], old_count);
    uint32_t byte = state->input_data[slot];
    bool complete = new_count == state->input_num_bytes_to_transfer[slot];

    state->input_data_valid[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_DATA_VALID_CONSUMED_CLEAR);
    state->input_byte_count[slot] = new_count;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_BYTE_COUNT_INCREMENT);
    state->dbuff_reg_word4 |= byte << shift;
    observe(observer, observer_context, CHANNEL_VBC_STORE_DBUFF_WORD4_OR_BYTE);
    state->input_ack[slot] = 1u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_ACK_SET);
    if (complete) {
        state->input_channel_status[slot] = CHANNEL_VBC_INPUT_STATUS_19;
        observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_STATUS_UPDATE);
        /* VBC continues through a priority-dependent tail not modeled here. */
        return CHANNEL_VBC_INPUT_BYTE_CONSUMED_STATUS_CHANGED;
    }
    return CHANNEL_VBC_INPUT_BYTE_CONSUMED;
}

static channel_vbc_input_consume_result_t consume_status_16(
    channel_vbc_state_t *state, uint32_t slot,
    channel_vbc_store_observer_fn observer, void *observer_context)
{
    uint32_t old_count = state->input_byte_count[slot];
    uint32_t new_count = old_count + 1u;
    uint32_t shift = input_shift_for_byte(state->input_byte_offset[slot], old_count);
    uint32_t byte = state->input_data[slot];

    state->input_data_valid[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_DATA_VALID_CONSUMED_CLEAR);
    state->input_byte_count[slot] = new_count;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_BYTE_COUNT_INCREMENT);
    state->input_ack[slot] = 1u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_ACK_SET);
    state->dbuff_reg_word4 |= byte << shift;
    observe(observer, observer_context, CHANNEL_VBC_STORE_DBUFF_WORD4_OR_BYTE);
    if (new_count == 4u) {
        state->input_channel_status[slot] = CHANNEL_VBC_INPUT_STATUS_18;
        observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_STATUS_UPDATE);
        /* VBC continues through a priority-dependent tail not modeled here. */
        return CHANNEL_VBC_INPUT_BYTE_CONSUMED_STATUS_CHANGED;
    }
    return CHANNEL_VBC_INPUT_BYTE_CONSUMED;
}

static channel_vbc_input_consume_result_t consume_status_1b(
    channel_vbc_state_t *state, uint32_t slot,
    channel_vbc_store_observer_fn observer, void *observer_context)
{
    uint32_t old_count = state->input_byte_count[slot];
    uint32_t new_count = old_count + 1u;
    uint32_t offset = state->input_byte_offset[slot];
    uint32_t shift = input_shift_for_byte(offset, old_count);
    uint32_t byte = state->input_data[slot];
    bool status_complete =
        new_count == state->input_num_bytes_to_transfer[slot] ||
        (new_count == 1u && offset == 3u) ||
        (new_count == 2u && offset == 2u) ||
        (new_count == 3u && offset == 1u);

    state->input_data_valid[slot] = 0u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_DATA_VALID_CONSUMED_CLEAR);
    state->dbuff_reg_word4 |= byte << shift;
    observe(observer, observer_context, CHANNEL_VBC_STORE_DBUFF_WORD4_OR_BYTE);
    state->input_ack[slot] = 1u;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_ACK_SET);
    state->input_byte_count[slot] = new_count;
    observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_BYTE_COUNT_INCREMENT);
    if (status_complete) {
        state->input_channel_status[slot] = CHANNEL_VBC_INPUT_STATUS_1C;
        observe(observer, observer_context, CHANNEL_VBC_STORE_INPUT_STATUS_UPDATE);
        /* VBC continues through a priority-dependent tail not modeled here. */
        return CHANNEL_VBC_INPUT_BYTE_CONSUMED_STATUS_CHANGED;
    }
    return CHANNEL_VBC_INPUT_BYTE_CONSUMED;
}

channel_vbc_input_consume_result_t channel_vbc_consume_input_byte(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT ||
        state->input_data_valid[slot] != 1u ||
        state->input_num_bytes_to_transfer[slot] == 0u) {
        return CHANNEL_VBC_INPUT_CONSUME_REJECTED;
    }

    switch (state->input_channel_status[slot]) {
    case CHANNEL_VBC_INPUT_STATUS_17:
        return consume_status_17(state, slot, observer, observer_context);
    case CHANNEL_VBC_INPUT_STATUS_16:
        return consume_status_16(state, slot, observer, observer_context);
    case CHANNEL_VBC_INPUT_STATUS_1B:
        return consume_status_1b(state, slot, observer, observer_context);
    default:
        return CHANNEL_VBC_INPUT_CONSUME_REJECTED;
    }
}

bool channel_vbc_service_input_ack_slot(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT ||
        state->input_ack[slot] != 1u) {
        return false;
    }

    /* x86 C8A1/C980/C987: InputAck==1 clears WaitingForInputAck,
     * then InputAck. These are VBC-emulator flags, not physical ACKs.
     */
    state->waiting_for_input_ack[slot] = 0u;
    observe(observer, observer_context,
            CHANNEL_VBC_STORE_WAITING_INPUT_ACK_REARM_CLEAR);
    state->input_ack[slot] = 0u;
    observe(observer, observer_context,
            CHANNEL_VBC_STORE_INPUT_ACK_REARM_CLEAR);
    return true;
}

bool channel_vbc_service_sending_input_ack_slot(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context)
{
    if (state == 0 || slot >= CHANNEL_VBC_SLOT_COUNT ||
        state->sending_input_ack[slot] != 1u ||
        state->link_out_serial_bus_usage_status[slot] != 0u) {
        return false;
    }

    /* x86 C960/C96C/C972, preceding the InputAck check in the normal path. */
    if (state->sending_input_ack[slot] == 1u &&
        state->link_out_serial_bus_usage_status[slot] == 0u) {
        state->link_out_serial_bus_usage_status[slot] = 2u;
        observe(observer, observer_context,
                CHANNEL_VBC_STORE_LINK_OUT_USAGE_INPUT_ACK_SET);
        state->sending_input_ack[slot] = 0u;
        observe(observer, observer_context,
                CHANNEL_VBC_STORE_SENDING_INPUT_ACK_REARM_CLEAR);
    }
    return true;
}
