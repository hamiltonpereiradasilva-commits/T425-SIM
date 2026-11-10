#include <stdio.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdlib.h>

#include "t425_core.h"
#include "t425_process.h"
#include "t425_memory.h"

static void fail(const char *msg)
{
    fprintf(stderr, "FAIL: %s\n", msg);
    exit(1);
}

static void expect_u32(const char *name, uint32_t got, uint32_t expected)
{
    if (got != expected) {
        fprintf(stderr,
                "FAIL: %s: got 0x%08X expected 0x%08X\n",
                name, got, expected);
        exit(1);
    }
}

static void write_word(
    t425_core_t *core,
    uint32_t address,
    uint32_t value)
{
    if (t425_memory_write_word(&core->memory, address, value) != 0)
        fail("memory write");
}

static uint32_t read_word(
    t425_core_t *core,
    uint32_t address)
{
    uint32_t value;

    if (t425_memory_read_word(&core->memory, address, &value) != 0)
        fail("memory read");

    return value;
}

static void run_pfix_opr(
    t425_core_t *core,
    uint8_t pfix,
    uint8_t opr)
{
    uint32_t iptr = core->cpu.IptrReg;

    /*
     * Encode the complete Transputer instruction bytes.
     *
     * The helper arguments are opcode nibbles:
     *   PFIX = 0x2
     *   OPR  = 0xF
     *
     * Therefore the memory bytes are:
     *   (PFIX << 4) | pfix
     *   (OPR  << 4) | opr
     */
    uint8_t pfix_byte =
        (uint8_t)((0x2u << 4) | pfix);
    uint8_t opr_byte =
        (uint8_t)((0xFu << 4) | opr);

    if (t425_memory_write_byte(
            &core->memory,
            iptr,
            pfix_byte) != 0)
        fail("PFIX program write");

    if (t425_memory_write_byte(
            &core->memory,
            iptr + 1u,
            opr_byte) != 0)
        fail("OPR program write");

    if (t425_core_step(core) != T425_STEP_OK)
        fail("PFIX");

    
    {
        int rc = t425_core_step(core);

        
        if (rc != T425_STEP_OK)
            fail("OPR");
    }

    }

int main(void)
{
    t425_core_t core;

    const uint32_t WA = T425_INTERNAL_RAM_BASE + 0x100u;
    const uint32_t WB = T425_INTERNAL_RAM_BASE + 0x180u;

    const uint32_t WDESC_A = WA | T425_PRIORITY_LOW;
    const uint32_t WDESC_B = WB | T425_PRIORITY_LOW;

    const uint32_t IP_A = T425_INTERNAL_RAM_BASE + 0x220u;
    const uint32_t IP_B = T425_INTERNAL_RAM_BASE + 0x260u;

    const uint32_t A_VALUE = 0x11223344u;
    const uint32_t B_VALUE = 0x55667788u;
    const uint32_t C_VALUE = 0x99AABBCCu;
    const uint32_t E_VALUE = 0x12345678u;
    const uint32_t STATUS_VALUE = 0xCAFEBABEu;

    t425_core_reset(&core);

    /*
     * Process A architectural descriptor.
     */
    write_word(&core, WA - 4u, IP_A);
    write_word(&core, WA - 8u, T425_NOT_PROCESS);
    write_word(&core, WA - 12u, T425_PROCESS_STATE_READY);
    write_word(&core, WA - 16u, T425_TIMER_TIME_NO_SET);
    write_word(&core, WA - 20u, 0u);

    /*
     * Process B is the process that continues while A waits.
     */
    write_word(&core, WB - 4u, IP_B);
    write_word(&core, WB - 8u, T425_NOT_PROCESS);
    write_word(&core, WB - 12u, T425_PROCESS_STATE_READY);
    write_word(&core, WB - 16u, T425_TIMER_TIME_NO_SET);
    write_word(&core, WB - 20u, 0u);

    /*
     * Put A and B on the ready queue.
     */
    core.cpu.WdescReg = WDESC_A;
    core.cpu.WptrReg = WA;
    core.cpu.IptrReg = IP_A;

    /*
     * Establish A as the currently running process.
     *
     * F08.2-G exercises ALT/timer/context integration; the
     * dispatch mechanism itself is covered by F08.2-D.
     * Keep the scheduler metadata coherent with the CPU
     * architectural process state, as done by the F08.2-D
     * process fixture.
     */
    /*
     * Establish A as the running process.
     *
     * F08.2-G exercises ALT/timer/context integration; the
     * dispatch mechanism itself is covered by F08.2-D.
     */
    core.scheduler.current_wdesc = WDESC_A;
    core.scheduler.running = 1u;
    if (t425_scheduler_enqueue(
            &core.scheduler,
            &core.memory,
            WDESC_A) != 0)
        fail("enqueue A");

    if (t425_scheduler_enqueue(
            &core.scheduler,
            &core.memory,
            WDESC_B) != 0)
        fail("enqueue B");

    /*
     * Consume A from the ready queue to model the initial
     * dispatch. B remains ready for the TALTWT context switch.
     */
    {
        uint32_t dispatched_wdesc = T425_NOT_PROCESS;
        uint32_t dispatched_iptr = 0u;

        if (t425_scheduler_dequeue(
                &core.scheduler,
                &core.memory,
                &dispatched_wdesc,
                &dispatched_iptr) != 0)
            fail("initial dispatch dequeue");

        expect_u32(
            "initial dispatched Wdesc",
            dispatched_wdesc,
            WDESC_A);

        expect_u32(
            "initial dispatched Iptr",
            dispatched_iptr,
            IP_A);
    }

    /*
     * Dispatch A.
     */
    if (t425_core_step(&core) != T425_STEP_OK)
        fail("initial dispatch");

    /*
     * Establish an architectural execution context in A.
     */
    core.cpu.A = A_VALUE; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = B_VALUE; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = C_VALUE; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Ereg = E_VALUE;
    core.cpu.StatusReg = STATUS_VALUE;

    /*
     * Current low timer = 100.
     */
    core.timer.low = 100u;
    core.timer.running = 1u;

    /*
     * TALT.
     */
    
    run_pfix_opr(&core, 4u, 0xEu);

    
    /*
     * ENBT with deadline 200.
     * ENBT consumes B as the deadline operand.
     */
    core.cpu.B = 200u; t425_fixture_sync_slaves(&core.cpu);

    
    run_pfix_opr(&core, 4u, 7u);

    
    /*
     * Restore the context values that must survive the wait.
     */
    core.cpu.A = A_VALUE; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = B_VALUE; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = C_VALUE; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Ereg = E_VALUE;
    core.cpu.StatusReg = STATUS_VALUE;

    /*
     * TALTWT.
     *
     * Capture the complete architectural timer/process state
     * immediately before and after the operation.
     */
    {
        uint32_t dbg_state;
        uint32_t dbg_tlink;
        uint32_t dbg_time;

        if (t425_memory_read_word(
                &core.memory,
                core.cpu.WptrReg - 12u,
                &dbg_state) != 0)
            fail("debug pre-state");

        if (t425_memory_read_word(
                &core.memory,
                core.cpu.WptrReg - 16u,
                &dbg_tlink) != 0)
            fail("debug pre-tlink");

        if (t425_memory_read_word(
                &core.memory,
                core.cpu.WptrReg - 20u,
                &dbg_time) != 0)
            fail("debug pre-time");

        
            }

    run_pfix_opr(&core, 5u, 1u);

    {
        uint32_t dbg_state;
        uint32_t dbg_tlink;
        uint32_t dbg_time;

        if (t425_memory_read_word(
                &core.memory,
                core.cpu.WptrReg - 12u,
                &dbg_state) != 0)
            fail("debug post-state");

        if (t425_memory_read_word(
                &core.memory,
                core.cpu.WptrReg - 16u,
                &dbg_tlink) != 0)
            fail("debug post-tlink");

        if (t425_memory_read_word(
                &core.memory,
                core.cpu.WptrReg - 20u,
                &dbg_time) != 0)
            fail("debug post-time");

            }

    /*
     * A must now be waiting and B must have been dispatched.
     */
    expect_u32(
        "A state after TALTWT",
        read_word(&core, WA - 12u),
        T425_PROCESS_STATE_WAITING);

    /*
     * TALTWT blocks after the instruction has been fetched.
     * The saved process IP therefore points to the continuation
     * address after the PFIX/OPR encoding.
     */
    expect_u32(
        "saved A Iptr",
        read_word(&core, WA - 4u),
        0x80000227u);
    expect_u32(
        "timer queue head",
        read_word(&core, T425_TPTR_LOC1),
        WDESC_A);

    expect_u32(
        "current Wdesc after A waits",
        core.cpu.WdescReg,
        WDESC_B);

    expect_u32(
        "current Wptr after A waits",
        core.cpu.WptrReg,
        WB);

    expect_u32(
        "current Iptr after A waits",
        core.cpu.IptrReg,
        IP_B);

    /*
     * B remains the running process while A waits for the timer.
     *
     * TALTWT consumed 48 processor cycles. LOW timer ticks require
     * 1280 processor cycles, therefore 1232 additional cycles
     * complete exactly one LOW timer tick.
     */
    core.timer.low = 199u;
    t425_timer_advance(&core.timer, 1232u);

    /*
     * B executes LDPRI. LDPRI uses finish(), which invokes the
     * timer service after advancing one processor cycle.
     *
     * PFIX 1 + LDPRI = 0x21 0xFE.
     */
    if (t425_memory_write_byte(
            &core.memory, IP_B, 0x21u) != 0 ||
        t425_memory_write_byte(
            &core.memory, IP_B + 1u, 0xFEu) != 0)
        fail("B LDPRI program write");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("B PFIX before LDPRI");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("B LDPRI timer service");

    /*
     * The timer must have reached/passed A's deadline and moved
     * A back to READY.
     */
    expect_u32(
        "A state after timer expiry",
        read_word(&core, WA - 12u),
        T425_PROCESS_STATE_READY);

    expect_u32(
        "A removed from timer queue",
        read_word(&core, T425_TPTR_LOC1),
        T425_NOT_PROCESS);

    /*
     * A must now be present in the ready scheduler.
     */
    expect_u32(
        "ready queue contains A",
        core.scheduler.fptr[T425_PRIORITY_LOW],
        WDESC_A);

    /*
     * B explicitly releases the processor through STOPP.
     * STOPP saves B and invokes the central scheduler.
     *
     * PFIX 1 + STOPP = 0x21 0xF5.
     */
    if (t425_memory_write_byte(
            &core.memory, IP_B + 2u, 0x21u) != 0 ||
        t425_memory_write_byte(
            &core.memory, IP_B + 3u, 0xF5u) != 0)
        fail("B STOPP program write");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("B PFIX before STOPP");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("STOPP dispatch A");

    expect_u32("restored Wdesc A", core.cpu.WdescReg, WDESC_A);
    expect_u32("restored Wptr A", core.cpu.WptrReg, WA);
    /*
     * TALTWT saves the continuation Iptr after the TALTWT
     * instruction has already been fetched.  The observed and
     * architectural resume address is therefore IP_A + 7.
     */
    expect_u32(
        "restored Iptr A",
        core.cpu.IptrReg,
        IP_A + 7u);
    expect_u32("restored A register", core.cpu.A, A_VALUE);
    expect_u32("restored B register", core.cpu.B, B_VALUE);
    expect_u32("restored C register", core.cpu.C, C_VALUE);
    expect_u32("restored Ereg", core.cpu.Ereg, E_VALUE);
    expect_u32("restored Status", core.cpu.StatusReg, STATUS_VALUE);

    puts("F08.2-G ALT/TIMER context wakeup: PASS");
    puts("F08.2-G WAITING -> READY -> dispatch -> context restore: PASS");
    return 0;
}
