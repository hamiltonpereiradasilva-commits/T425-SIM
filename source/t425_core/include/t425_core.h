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

#define T425_MAX_PROCESS_CONTEXTS 32u

/*
 * Normal T425 process context.
 *
 * This represents the architectural register state associated with
 * a suspended process. It is distinct from the interrupt-save context.
 */
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
    uint32_t t425_data_mem_addr;
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
     * previous_read_data is the result produced by MemRead.
     * data_in is the DataIn interface value selected by the
     * following microinstruction through DataInfromPrevReadReg.
     */
    uint32_t previous_read_data;
    uint32_t data_in;

    /* F20-10-05D — physical DataInReg. */
    uint32_t data_in_reg;

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

    t425_datapath_t datapath;
    t425_memory_t memory;
    t425_scheduler_state_t scheduler;
    t425_timer_state_t timer;
    uint64_t cycles;
    uint64_t instructions;
} t425_core_t;

#define T425_STEP_OK        0
#define T425_STEP_HALTED    1
#define T425_STEP_BADMEM   -1
#define T425_STEP_UNIMPL   -2
#define T425_STEP_BADOP    -3

void t425_core_reset(t425_core_t *core);

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

#endif
