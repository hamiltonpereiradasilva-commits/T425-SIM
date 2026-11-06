#ifndef T425_CHANNEL_VBC_MODEL_REQ03_06_H
#define T425_CHANNEL_VBC_MODEL_REQ03_06_H

#include <stdbool.h>
#include <stdint.h>

#define CHANNEL_VBC_SLOT_COUNT 4u
#define CHANNEL_VBC_DIRECTIONAL_COUNT 8u
#define CHANNEL_VBC_INPUT_B_ALT_STATUS 0x14u
#define CHANNEL_VBC_INPUT_ALT_READY_STATUS 0x15u
#define CHANNEL_VBC_INPUT_SLOT0_SELECT 0x80000010u
#define CHANNEL_VBC_INPUT_STATUS_16 0x16u
#define CHANNEL_VBC_INPUT_STATUS_17 0x17u
#define CHANNEL_VBC_INPUT_STATUS_18 0x18u
#define CHANNEL_VBC_INPUT_STATUS_19 0x19u
#define CHANNEL_VBC_INPUT_STATUS_1B 0x1Bu
#define CHANNEL_VBC_INPUT_STATUS_1C 0x1Cu

typedef enum {
    CHANNEL_VBC_STORE_INPUT_DATA_VALID_SET = 1,
    CHANNEL_VBC_STORE_LINK_IN_USAGE_CLEAR,
    CHANNEL_VBC_STORE_WAITING_INPUT_ACK_SET,
    CHANNEL_VBC_STORE_SENDING_INPUT_ACK_SET,
    CHANNEL_VBC_STORE_INPUT_DATA_COPY,
    CHANNEL_VBC_STORE_WAITING_OUTPUT_ACK_CLEAR,
    CHANNEL_VBC_STORE_OUTPUT_DATA_VALID_CLEAR,
    CHANNEL_VBC_STORE_LINK_OUT_USAGE_CLEAR,
    CHANNEL_VBC_STORE_SENDING_INPUT_ACK_CLEAR,
    CHANNEL_VBC_STORE_WAITING_INPUT_ACK_CLEAR,
    CHANNEL_VBC_STORE_INPUT_DATA_VALID_CLEAR,
    CHANNEL_VBC_STORE_LINK_IN_USAGE_CLEAR_RESET,
    CHANNEL_VBC_STORE_INPUT_ACK_CLEAR,
    CHANNEL_VBC_STORE_INPUT_DATA_VALID_CONSUMED_CLEAR,
    CHANNEL_VBC_STORE_INPUT_BYTE_COUNT_INCREMENT,
    CHANNEL_VBC_STORE_DBUFF_WORD4_OR_BYTE,
    CHANNEL_VBC_STORE_INPUT_ACK_SET,
    CHANNEL_VBC_STORE_INPUT_STATUS_UPDATE,
    CHANNEL_VBC_STORE_LINK_OUT_USAGE_INPUT_ACK_SET,
    CHANNEL_VBC_STORE_SENDING_INPUT_ACK_REARM_CLEAR,
    CHANNEL_VBC_STORE_WAITING_INPUT_ACK_REARM_CLEAR,
    CHANNEL_VBC_STORE_INPUT_ACK_REARM_CLEAR
} channel_vbc_store_event_t;

typedef enum {
    CHANNEL_VBC_INPUT_CONSUME_REJECTED = 0,
    CHANNEL_VBC_INPUT_BYTE_CONSUMED = 1,
    CHANNEL_VBC_INPUT_BYTE_CONSUMED_STATUS_CHANGED = 2
} channel_vbc_input_consume_result_t;

typedef void (*channel_vbc_store_observer_fn)(
    void *context,
    channel_vbc_store_event_t event);

typedef struct {
    uint32_t input_channel_status[CHANNEL_VBC_SLOT_COUNT];
    uint32_t input_data_valid[CHANNEL_VBC_SLOT_COUNT];
    uint32_t alt_enabled_latch[CHANNEL_VBC_SLOT_COUNT];
    uint32_t input_ready_req[CHANNEL_VBC_SLOT_COUNT];
    uint32_t input_run_req[CHANNEL_VBC_SLOT_COUNT];
    uint32_t output_run_req[CHANNEL_VBC_SLOT_COUNT];
    uint32_t subsystem_enable_latch[CHANNEL_VBC_DIRECTIONAL_COUNT];
    uint32_t count_reg[CHANNEL_VBC_DIRECTIONAL_COUNT];
    uint32_t link_pri_reg[CHANNEL_VBC_DIRECTIONAL_COUNT];
    uint32_t input_pri_out[CHANNEL_VBC_SLOT_COUNT];
    uint32_t ptr_reg[CHANNEL_VBC_DIRECTIONAL_COUNT];
    uint32_t select_channel_reg;
    uint32_t warnings;
    uint8_t mir_bytes[10];

    /* Per-slot state used by the recovered VBC link-interface producer. */
    uint32_t channel_selected[CHANNEL_VBC_SLOT_COUNT];
    uint32_t link_in_serial_bus_usage_status[CHANNEL_VBC_SLOT_COUNT];
    uint8_t link_in_serial_bus_payload[CHANNEL_VBC_SLOT_COUNT];
    uint32_t waiting_for_input_ack[CHANNEL_VBC_SLOT_COUNT];
    uint32_t sending_input_ack[CHANNEL_VBC_SLOT_COUNT];
    uint8_t input_data[CHANNEL_VBC_SLOT_COUNT];

    /* Additional state cleared by the recovered selected-link reset. */
    uint32_t waiting_for_output_ack[CHANNEL_VBC_SLOT_COUNT];
    uint32_t output_data_valid[CHANNEL_VBC_SLOT_COUNT];
    uint32_t link_out_serial_bus_usage_status[CHANNEL_VBC_SLOT_COUNT];
    uint32_t input_ack[CHANNEL_VBC_SLOT_COUNT];

    /* INPUT byte-consumer state recovered from VBC x86 handler blocks. */
    uint32_t input_num_bytes_to_transfer[CHANNEL_VBC_SLOT_COUNT];
    uint32_t input_byte_count[CHANNEL_VBC_SLOT_COUNT];
    uint32_t input_byte_offset[CHANNEL_VBC_SLOT_COUNT];
    uint32_t dbuff_reg_word4;
} channel_vbc_state_t;

/* Existing REQ-03 isolated INPUT-B/ALT slot-0 transition. */
/* F57: set only after a validated ACK commit and terminal CountReg. */
bool channel_vbc_output_terminal_ack(channel_vbc_state_t *state, uint32_t slot,
                                     uint32_t count_post);
bool channel_vbc_output_gate_set(channel_vbc_state_t *state, uint32_t slot);
bool channel_vbc_output_gate_clear(channel_vbc_state_t *state, uint32_t slot);
uint8_t channel_vbc_output_run_request_reduce(
    const channel_vbc_state_t *state);

bool channel_vbc_input_b_alt_slot0(channel_vbc_state_t *state);

/* Existing REQ-03 selected slot-0 clear; raw MIR gates are preserved. */
bool channel_vbc_clear_selected_input_slot0(channel_vbc_state_t *state);

/* Recovered _HandleLinkInterfaces positive publication, for one slot i.
 * Optional observer exposes store order for tests; it does not affect state.
 */
bool channel_vbc_publish_input_data_valid(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context);

/* Recovered _HandleLinkInterfaces selected reset. MIR byte 9 and
 * ChannelSelected[i] are raw gates; no physical/MIR semantic is inferred.
 */
bool channel_vbc_reset_selected_link_interface_slot(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context);

/* Consume one published byte for recovered INPUT statuses 0x16/0x17/0x1B.
 * Models only identified data/count/ACK/status stores; it does not rearm
 * WaitingForInputAck or execute the handler's subsequent priority tail.
 */
channel_vbc_input_consume_result_t channel_vbc_consume_input_byte(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context);

/* Block-level stores from _HandleLinkInterfaces. Call only when static CFG
 * guards have reached the corresponding VBC blocks; no physical ACK is modeled.
 */
bool channel_vbc_service_sending_input_ack_slot(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context);

bool channel_vbc_service_input_ack_slot(
    channel_vbc_state_t *state,
    uint32_t slot,
    channel_vbc_store_observer_fn observer,
    void *observer_context);

#endif
