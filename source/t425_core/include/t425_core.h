#ifndef T425_CORE_H
#define T425_CORE_H

#include <stdint.h>

#include "t425_cpu.h"
#include "t425_datapath.h"
#include "t425_microsequencer.h"
#include "t425_microentry.h"
#include "t425_memory.h"
#include "t425_scheduler.h"
#include "t425_timer.h"
#include "t425_core_phy.h"
#include "t425_channel_dma_foundation_01.h"
#include "t425_channel_admission_21.h"
#include "t425_channel_dma_shift_05.h"
#include "t425_channel_dma_rx_adapter_10.h"
#include "t425_channel_vbc_model_req03_06.h"

#define T425_MAX_PROCESS_CONTEXTS 32u

struct t425_channel_compat_rx_adapter_07;

/*
 * Normal T425 process context.
 *
 * This represents the architectural register state associated with
 * a suspended process. It is distinct from the interrupt-save context.
 */
typedef enum
{
    T425_BOOT_RESET = 0,
    T425_BOOT_DECIDE,
    T425_BOOT_WAIT_CONTROL,
    T425_BOOT_POKE_ADDRESS,
    T425_BOOT_POKE_DATA,
    T425_BOOT_PEEK_ADDRESS,
    T425_BOOT_PEEK_OUTPUT,
    T425_BOOT_LOAD,
    T425_BOOT_RUNNING
} t425_boot_state_t;

typedef struct
{
    uint8_t valid;

    /* Simulator lookup key / workspace association. */
    uint32_t wptr;

    /* Normal T425 process architectural context. */
    uint32_t Wdesc;
    uint32_t Iptr;
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t Ereg;
    uint32_t Status;
} t425_process_context_t;

/*
 * T425 interrupt-save context.
 *
 * This is deliberately separate from the normal process context.
 */
typedef struct
{
    uint8_t valid;

    uint32_t WdescIntSave;
    uint32_t IptrIntSave;
    uint32_t AregSave;
    uint32_t BregSave;
    uint32_t CregSave;
    uint32_t StatusIntSave;
    uint32_t EregIntSave;
} t425_interrupt_context_t;

#define T425_MAX_INTERRUPT_CONTEXTS 32u

/*
 * Simulator-internal state for an architectural IN/OUT rendezvous.
 *
 * This is not a FIFO and does not contain message data.  The channel
 * control word in T425 memory remains authoritative for the waiting
 * process.  This state records only the information required to complete
 * the rendezvous when the complementary IN/OUT operation arrives.
 */
typedef struct
{
    /* T425 VBC internal channel machinery. */
    uint32_t t425_channel_select;
    uint32_t t425_channel_stack;
    t425_channel_dma_bank_t t425_channel_dma;
    t425_channel_admission_21_t channel_admission;
    /* F05 uses the same F17 bank and this CORE instance memory. */
    t425_channel_dma_memory_04_t channel_dma_memory;
    t425_channel_dma_shift_05_t channel_dma_shift;
    t425_channel_dma_rx_adapter_10_t channel_dma_rx_adapter_10;
    uint8_t channel_dma_rx_adapter_10_initialized;
    /* Optional REA-08 callbacks; NULL in the legacy/default profile. */
    void *channel_compat_profile_08_context;
    void (*channel_compat_profile_08_microstep)(void *context, void *core);
    uint8_t (*channel_compat_profile_08_reduce)(void *context);
    bool (*channel_compat_profile_08_clear)(
        void *context, uint32_t selected_channel, uint16_t mir_control);
    bool (*channel_compat_profile_08_service)(void *context, void *core);
    uint32_t t425_data_mem_addr;
    uint8_t experimental_data_mem_addr_from_wdesc;
    uint32_t t425_data_reg;

    /*
     * T425 VBC channel-unit state.
     *
     * ChannelDataReg is distinct from the general DataReg.
     * The following fields represent channel control state exposed
     * by the VBC channel machinery. Exact physical storage granularity
     * is intentionally not assumed here.
     */
    uint32_t t425_channel_data_reg;
    uint32_t clock_reg0;
    uint32_t clock_reg1;
    uint8_t  t425_channel_ready_not_run;
    uint8_t  t425_channel_run_req;
    uint8_t  t425_channel_status;
    uint8_t  t425_alt_enabled;

    /* F57: integrated REQ03 channel request/gate state. */
    channel_vbc_state_t channel_vbc_state;

    /* Non-owning optional portable CORE PHY attachment. */
    t425_core_phy_t *phy;

    /* F_CORE-SYS-01 reset/bootstrap state. */
    uint8_t reset_active;
    uint8_t bootfromrom_sampled;
    uint8_t bootfromrom_value;
    t425_boot_state_t boot_state;
    uint8_t bootstrap_link;
    uint32_t bootstrap_count;
    uint32_t bootstrap_index;
    uint32_t bootstrap_address;
    uint32_t bootstrap_word;
    uint8_t bootstrap_byte_index;
    uint32_t rx_credit_generation[T425_CORE_PHY_MAX_ENDPOINTS];
    uint8_t rx_credit_enabled[T425_CORE_PHY_MAX_ENDPOINTS];
    uint8_t rx_credit_rearm[T425_CORE_PHY_MAX_ENDPOINTS];

    /*
     * Explicit lifetime for an architectural external IN transfer.
     * The channel control word remains authoritative for rendezvous;
     * these fields only retain the byte-transfer lifetime after TAKE.
     */
    uint8_t external_in_active[T425_CORE_PHY_MAX_ENDPOINTS];
    uint8_t external_in_seen[T425_CORE_PHY_MAX_ENDPOINTS];
    uint32_t external_in_remaining[T425_CORE_PHY_MAX_ENDPOINTS];

    /*
     * Simulator-internal register context for processes which are
     * not currently executing. This is not architectural RAM and
     * is not channel state.
     */
    t425_process_context_t process_contexts[T425_MAX_PROCESS_CONTEXTS];

    /*
     * Separate simulator representation for T425 interrupt-save state.
     * It must never be conflated with the normal process context.
     */
    t425_interrupt_context_t
        interrupt_contexts[T425_MAX_INTERRUPT_CONTEXTS];

    /*
     * Pending IN/OUT rendezvous descriptors.
     * No message FIFO is represented here.
     */

    t425_cpu_state_t cpu;
    t425_microsequencer_t microsequencer;
    uint16_t microentry_signal;

    /*
     * Established T425 instruction/microcode execution state.
     *
     * StepInstruction:
     *     current instruction is executing through microcode.
     *
     * nextStepInstruction:
     *     state requested by the current microinstruction for
     *     the next core step.
     */
    uint8_t step_instruction;
    uint8_t next_step_instruction;

    /*
     * T425 memory-interface read pipeline.
     *
     * data_in_reg is the authoritative functional DataInReg consumed
     * by XBUS and ByteUpShifter. data_in is a public compatibility
     * mirror synchronized by the CORE at interface commits.
     * previous_read_data is the separate PreviousReadReg value.
     */
    uint32_t previous_read_data;
    uint32_t data_in;

    /* F20-10-05D — physical DataInReg. */
    uint32_t data_in_reg;

    /* Experimental delayed MemRead pipeline for F_CORE_PROCESS_RESUME_04. */
    uint8_t memory_read_pending;
    uint8_t memory_read_pending_areg;
    uint32_t memory_read_pending_address;

    /* F20-10-05D — physical TwoReadsLatch. */
    uint8_t two_reads_latch;

    /* F20-10-05D — ClearBWSELatchAfterWrite event. */
    uint8_t clear_bwse_latch_after_write;

    /*
     * F20-09 — persistent ByteWriteStrobeEnable latch.
     *
     * Its set/reset semantics are handled separately in F20-10.
     */
    uint8_t byte_write_strobe_latch;

    /* F45: byte transaction staged for the next MemoryInterface boundary. */
    uint8_t byte_write_pending;
    uint8_t byte_write_pending_mask;
    uint8_t byte_write_pending_clear_latch;
    uint32_t byte_write_pending_address;
    uint32_t byte_write_pending_data;
    uint32_t byte_write_pending_iptr;
    uint16_t byte_write_pending_mpc;

    t425_datapath_t datapath;
    t425_memory_t memory;
    t425_scheduler_state_t scheduler;
    t425_timer_state_t timer;
    uint64_t cycles;
    uint64_t instructions;
} t425_core_t;

/* Service ACTIVE F28 -> F10/F05 RX and REQUEST_PENDING -> F05 handoff. */
bool t425_core_channel_dma_service(t425_core_t *core);
bool t425_core_channel_compat_profile_08_enable(
    t425_core_t *core, struct t425_channel_compat_rx_adapter_07 *adapter,
    const uint8_t lane_ids[4]);

bool t425_core_channel_admission_configure_route(
    t425_core_t *core, uint32_t architectural_channel,
    uint8_t endpoint, uint8_t physical_link);
bool t425_core_channel_admission_get_request(
    const t425_core_t *core, uint32_t architectural_channel,
    t425_channel_admission_request_21_t *request);
/* Validate and snapshot an already ACTIVE F21/F05 session without mutation.
 * Expected route and operation identity are supplied by the caller so stale
 * sessions or a changed endpoint/link route fail closed. */
bool t425_core_channel_admission_get_active_compat_input(
    const t425_core_t *core, uint32_t architectural_channel,
    uint32_t expected_request_id, uint32_t expected_operation_generation,
    uint8_t expected_endpoint, uint8_t expected_physical_link,
    t425_channel_admission_request_21_t *request);
bool t425_core_channel_admission_get_active(
    const t425_core_t *core, uint32_t architectural_channel,
    uint32_t expected_request_id, uint32_t expected_operation_generation,
    uint8_t expected_endpoint, uint8_t expected_physical_link,
    t425_channel_admission_request_21_t *request);
bool t425_core_channel_admission_report_begin(
    t425_core_t *core, uint32_t architectural_channel, uint32_t request_id,
    t425_channel_admission_begin_result_21_t result);
bool t425_core_channel_admission_report_complete_compat_input(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation);
bool t425_core_channel_admission_retire_compat_input(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation);
bool t425_core_channel_admission_report_complete(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation);
bool t425_core_channel_admission_retire(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation);
bool t425_core_channel_admission_abort(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation);

#define T425_STEP_OK        0
#define T425_STEP_HALTED    1
#define T425_STEP_BADMEM   -1
#define T425_STEP_UNIMPL   -2
#define T425_STEP_BADOP    -3

void t425_core_reset(t425_core_t *core);

bool t425_core_attach_phy(t425_core_t *core, t425_core_phy_t *phy);
int t425_core_link_output(t425_core_t *core, uint32_t link_id);
bool t425_core_link_input_available(t425_core_t *core, uint32_t link_id);
int t425_core_link_input_consume(
    t425_core_t *core, uint32_t link_id, uint8_t *byte);
bool t425_core_link_rx_admission(
    const t425_core_t *core, uint32_t link_id);

int t425_core_external_in_begin(
    t425_core_t *core, uint32_t link_id, uint32_t byte_count);

uint32_t t425_core_external_in_remaining(
    const t425_core_t *core, uint32_t link_id);

int t425_core_step(t425_core_t *core);

/*
 * Execute one T425 microcycle using the current MPC/MIR,
 * datapath and generated condition signals.
 */
void t425_core_microcycle(t425_core_t *core);

/*
 * Enter the T425 microcode machine from an instruction opcode.
 */
void t425_core_microentry(
    t425_core_t *core,
    uint8_t opcode);

/* Returns true when the system/bootstrap layer consumed this step. */
bool t425_core_system_step(t425_core_t *core);

#endif
