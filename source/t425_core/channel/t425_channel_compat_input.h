#ifndef T425_CHANNEL_COMPAT_INPUT_H
#define T425_CHANNEL_COMPAT_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define T425_CHANNEL_COMPAT_INPUT_LANES 4u
#define T425_CHANNEL_COMPAT_INPUT_PUSH_DEPTH 1u

typedef enum {
    T425_CHANNEL_COMPAT_INPUT_IDLE_14 = 0x14,
    T425_CHANNEL_COMPAT_INPUT_RECEIVE_16 = 0x16,
    T425_CHANNEL_COMPAT_INPUT_RECEIVE_17 = 0x17,
    T425_CHANNEL_COMPAT_INPUT_FULL_WORD_18 = 0x18,
    T425_CHANNEL_COMPAT_INPUT_PARTIAL_19 = 0x19,
    T425_CHANNEL_COMPAT_INPUT_POST_WRITE_1A = 0x1A,
    T425_CHANNEL_COMPAT_INPUT_UNALIGNED_1B = 0x1B,
    T425_CHANNEL_COMPAT_INPUT_UNALIGNED_COMMIT_1C = 0x1C
} t425_channel_compat_input_state_t;

typedef struct {
    uint32_t architectural_channel;
    uint8_t input_lane;
    uint8_t stack_lane;
    uint8_t shared_index;
    uint8_t endpoint_id;
    uint8_t physical_link;
    uint8_t lane_id;
} t425_channel_compat_input_route_t;

typedef struct {
    t425_channel_compat_input_route_t route;
    uint32_t request_id;
    uint32_t generation;
} t425_channel_compat_input_key_t;

typedef enum {
    T425_CHANNEL_COMPAT_INPUT_OFFER_ACCEPTED = 0,
    T425_CHANNEL_COMPAT_INPUT_OFFER_BACKPRESSURE,
    T425_CHANNEL_COMPAT_INPUT_OFFER_STALE_SESSION,
    T425_CHANNEL_COMPAT_INPUT_OFFER_INVALID
} t425_channel_compat_input_offer_result_t;

typedef enum {
    T425_CHANNEL_COMPAT_COMMIT_SUCCESS = 0,
    T425_CHANNEL_COMPAT_COMMIT_RETRY,
    T425_CHANNEL_COMPAT_COMMIT_ERROR
} t425_channel_compat_input_commit_status_t;

typedef struct {
    t425_channel_compat_input_key_t session;
    uint32_t ubus_request_id;
    uint32_t aligned_address;
    uint32_t write_data;
    uint8_t byte_mask;
    t425_channel_compat_input_commit_status_t status;
} t425_channel_compat_input_commit_receipt_t;

typedef enum {
    T425_CHANNEL_COMPAT_PUSH_PASS_THROUGH = 0,
    T425_CHANNEL_COMPAT_PUSH_STORED,
    T425_CHANNEL_COMPAT_PUSH_BUSY,
    T425_CHANNEL_COMPAT_PUSH_INVALID
} t425_channel_compat_input_push_result_t;

typedef enum {
    T425_CHANNEL_COMPAT_PUSH_ZBUS = 0,
    T425_CHANNEL_COMPAT_PUSH_TESTHARDCHAN
} t425_channel_compat_input_push_kind_t;

typedef struct {
    uint8_t valid;
    t425_channel_compat_input_push_kind_t kind;
    uint32_t value;
} t425_channel_compat_input_push_mailbox_t;

typedef struct {
    uint8_t active;
    uint8_t input_data_valid;
    uint8_t input_ack;
    uint8_t input_run_req;
    uint8_t input_pri_out;
    uint8_t input_complete;
    uint8_t input_data;
    uint8_t input_byte_offset;
    uint8_t input_byte_count;
    uint32_t input_num_bytes_to_transfer;
    t425_channel_compat_input_state_t input_state;
    t425_channel_compat_input_route_t route;
    uint32_t request_id;
    uint32_t generation;
    /* Experimental F123: persistent owner of input_run_req, independent of
     * the operational RX session key above. */
    uint8_t run_req_owner_valid;
    t425_channel_compat_input_key_t run_req_owner;
    uint32_t initial_count_reg;
    uint32_t initial_ptr_reg;
    uint32_t initial_dbuff_reg;
    uint32_t count_reg;
    uint32_t ptr_reg;
    uint32_t dbuff_reg;
    uint8_t ubus_request_pending;
    uint8_t ubus_byte_mask;
    uint32_t ubus_request_id;
    uint32_t next_ubus_request_id;
    uint32_t ubus_request_generation;
    uint32_t ubus_request_address;
    uint32_t ubus_request_data;
    t425_channel_compat_input_key_t ubus_session;
    uint8_t staging_mask;
    uint8_t last_commit_valid;
    uint8_t last_commit_mask;
    uint32_t last_commit_request_id;
    uint32_t last_commit_generation;
    uint32_t last_commit_address;
    uint32_t last_commit_data;
    t425_channel_compat_input_push_mailbox_t push_mailbox;
} t425_channel_compat_input_lane_t;

typedef enum {
    T425_CHANNEL_IFNEXT_OWNER_NONE = 0,
    T425_CHANNEL_IFNEXT_OWNER_SELECTED,
    T425_CHANNEL_IFNEXT_OWNER_AMBIGUOUS,
    T425_CHANNEL_IFNEXT_OWNER_PENDING
} t425_channel_ifnext_owner_result_t;

typedef struct {
    uint8_t subsystem_enable_latch[T425_CHANNEL_COMPAT_INPUT_LANES * 2u];
    t425_channel_compat_input_lane_t lane[T425_CHANNEL_COMPAT_INPUT_LANES];
    /* F123 sideband snapshot captured at the IF Next selection boundary. */
    uint8_t selected_owner_valid;
    t425_channel_compat_input_key_t selected_owner;
} t425_channel_compat_input_t;

/* Explicit address decode; endpoint and physical-link identities are not inferred. */
bool t425_channel_compat_input_decode_address(
    uint32_t architectural_channel, uint8_t *input_lane,
    uint8_t *stack_lane, uint8_t *shared_index);

/* Construct a route using externally configured endpoint/link identities. */
bool t425_channel_compat_input_route_from_config(
    uint32_t architectural_channel, uint8_t endpoint_id,
    uint8_t physical_link, uint8_t lane_id,
    t425_channel_compat_input_route_t *route);

/* Validate explicit route fields against architectural address decode. */
bool t425_channel_compat_input_route_valid(
    const t425_channel_compat_input_route_t *route);

void t425_channel_compat_input_init(t425_channel_compat_input_t *input);

/* Capture the immutable operation identity and post-push register snapshot. */
bool t425_channel_compat_input_capture_after_pushes(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_route_t *route,
    uint32_t request_id, uint32_t generation,
    uint8_t completed_push_count,
    uint32_t count_reg, uint32_t ptr_reg, uint32_t dbuff_reg,
    bool enable);

/* ACTIVE F28 has an aligned F17 PtrReg; the original byte offset comes from
 * the admitted buffer descriptor and remains an independent value. */
bool t425_channel_compat_input_capture_after_pushes_at_offset(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_route_t *route,
    uint32_t request_id, uint32_t generation,
    uint8_t completed_push_count,
    uint32_t count_reg, uint32_t ptr_reg, uint32_t dbuff_reg,
    uint8_t input_byte_offset, bool enable);

/* One-deep DATA publication. Does not mutate CountReg/PtrReg/DBuffReg. */
t425_channel_compat_input_offer_result_t
t425_channel_compat_input_offer_rx(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key, uint8_t byte);

/* Consume only an already-published byte into the isolated CHANNEL staging word. */
bool t425_channel_compat_input_consume_data(
    t425_channel_compat_input_t *input, uint8_t input_lane);

bool t425_channel_compat_input_key_matches(
    const t425_channel_compat_input_lane_t *lane,
    const t425_channel_compat_input_key_t *key);

/* Clear the internal ACK after the transport observes consumption. */
bool t425_channel_compat_input_acknowledge(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key);

typedef enum {
    T425_CHANNEL_COMPAT_INPUT_STEP_NO_ACTION = 0,
    T425_CHANNEL_COMPAT_INPUT_STEP_DATA_CONSUMED,
    T425_CHANNEL_COMPAT_INPUT_STEP_POST_WRITE,
    T425_CHANNEL_COMPAT_INPUT_STEP_COMPLETE,
    T425_CHANNEL_COMPAT_INPUT_STEP_INVALID
} t425_channel_compat_input_step_result_t;

/* One normal INPUT transition; never publishes RunReq to the CORE. */
t425_channel_compat_input_step_result_t t425_channel_compat_input_step(
    t425_channel_compat_input_t *input, uint8_t input_lane);

/* Set one pending masked-write identity from the active staging bytes. */
bool t425_channel_compat_input_prepare_commit(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key,
    uint32_t ubus_request_id, uint32_t aligned_address,
    uint32_t write_data, uint8_t byte_mask);

/* Form the next eligible 18/19/1C commit. RETRY retains this exact identity. */
bool t425_channel_compat_input_prepare_next_commit(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key);

typedef enum {
    T425_CHANNEL_COMPAT_INPUT_COMMIT_APPLIED = 0,
    T425_CHANNEL_COMPAT_INPUT_COMMIT_RETRY_PENDING,
    T425_CHANNEL_COMPAT_INPUT_COMMIT_DUPLICATE,
    T425_CHANNEL_COMPAT_INPUT_COMMIT_STALE,
    T425_CHANNEL_COMPAT_INPUT_COMMIT_ERROR,
    T425_CHANNEL_COMPAT_INPUT_COMMIT_INVALID
} t425_channel_compat_input_apply_result_t;

t425_channel_compat_input_apply_result_t
t425_channel_compat_input_apply_receipt(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_commit_receipt_t *receipt);

/* Stage an atomic push event while active; second event reports BUSY. */
t425_channel_compat_input_push_result_t
 t425_channel_compat_input_stage_push(
    t425_channel_compat_input_t *input, uint8_t input_lane,
    t425_channel_compat_input_push_kind_t kind, uint32_t value);

/* Return and clear the pending event only after the caller retires the session. */
bool t425_channel_compat_input_release_push_mailbox(
    t425_channel_compat_input_t *input, uint8_t input_lane,
    t425_channel_compat_input_push_mailbox_t *event);

/* Release identity/staging state; persistent request/priority latches survive. */
bool t425_channel_compat_input_retire(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key);

/* Strict retirement for a completed RX operation after F21 retirement. */
bool t425_channel_compat_input_retire_completed(
    t425_channel_compat_input_t *input,
    const t425_channel_compat_input_key_t *key);

/* OR reduction of the four INPUT requests with shared Enable indices 4..7. */
uint8_t t425_channel_compat_input_reduce_requests(
    const t425_channel_compat_input_t *input);

/* F124: inspect the unique candidate without changing the pending selection. */
t425_channel_ifnext_owner_result_t
t425_channel_compat_input_preview_unique_ifnext_owner(
    const t425_channel_compat_input_t *input, bool if_next,
    t425_channel_compat_input_key_t *candidate_owner);

/* Snapshot the unique eligible latch owner at IF Next. A different owner is
 * rejected while a previous handler/clear token remains outstanding. */
t425_channel_ifnext_owner_result_t
t425_channel_compat_input_bind_unique_ifnext_owner(
    t425_channel_compat_input_t *input, bool if_next,
    t425_channel_compat_input_key_t *selected_owner);

/* F123: generation-checked MIR ResetChannelRunReqlatch. */
bool t425_channel_compat_input_clear_selected_owned(
    t425_channel_compat_input_t *input, uint32_t architectural_channel,
    const t425_channel_compat_input_key_t *selected_owner,
    uint16_t mir_channel_control);

/* MIR ResetChannelRunReqlatch for selected, idle INPUT channel; F123 requires
 * a preceding unique IF Next owner snapshot. */
bool t425_channel_compat_input_clear_selected(
    t425_channel_compat_input_t *input, uint32_t architectural_channel,
    uint16_t mir_channel_control);

/* Disable only the selected architectural INPUT lane. RunReq and transfer
 * state are preserved; the caller owns any separate admission-state sync. */
bool t425_channel_compat_input_disable_selected(
    t425_channel_compat_input_t *input, uint32_t architectural_channel);

#endif
