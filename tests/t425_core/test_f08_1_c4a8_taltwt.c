#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "t425_core.h"
#include "t425_process.h"
#include <assert.h>

static int check_word(
    const t425_core_t *core,
    uint32_t address,
    uint32_t expected,
    const char *name)
{
    uint32_t value = 0u;

    if (t425_memory_read_word(&core->memory, address, &value) != 0)
    {
        fprintf(stderr, "%s: memory read failed\n", name);
        return 0;
    }

    if (value != expected)
    {
        fprintf(stderr,
                "%s: expected 0x%08x, got 0x%08x\n",
                name, expected, value);
        return 0;
    }

    return 1;
}

static void setup_process(
    t425_core_t *core,
    uint32_t wptr,
    uint32_t iptr,
    uint32_t priority)
{
    uint32_t wdesc = wptr | priority;

    (void)t425_memory_write_word(
        &core->memory, wptr - 4u, iptr);

    (void)t425_memory_write_word(
        &core->memory, wptr - 8u, T425_NOT_PROCESS);

    (void)t425_memory_write_word(
        &core->memory, wptr - 12u,
        T425_PROCESS_STATE_ENABLING);

    (void)t425_memory_write_word(
        &core->memory, wptr - 16u,
        T425_TIMER_TIME_SET);

    (void)t425_memory_write_word(
        &core->memory, wptr - 20u, 120u);

    core->scheduler.current_wdesc = wdesc;
    core->scheduler.running = 1u;
    core->cpu.Wptr = wptr;
    core->cpu.Iptr = iptr;
}

static int test_future_low(void)
{
    t425_core_t core;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    /*
     * Low-priority process.
     * Current low timer = 100, ALT time = 120.
     */
    setup_process(&core, 0x80000100u, 0x80000080u,
                  T425_PRIORITY_LOW);

    core.timer.low = 100u;

    core.cpu.Iptr = 0x80000080u;

    /*
     * TALTWT is OPR 0x51.
     *
     * Real instruction-stream encoding:
     *   0x25 = PFIX 5
     *   0xF1 = OPR 1
     *
     * PFIX forms Oreg = 0x50; OPR adds the final nibble,
     * producing Oreg = 0x51.
     */
    (void)t425_memory_write_byte(
        &core.memory, core.cpu.Iptr, 0x25u);
    (void)t425_memory_write_byte(
        &core.memory, core.cpu.Iptr + 1u, 0xF1u);

    /*
     * TALTWT is OPR 0x51 and is encoded as:
     *   PFIX 5 = 0x25
     *   OPR 1  = 0xF1
     *
     * Each byte is consumed by a separate core step.
     */
    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr, "C4-A.8 future low: PFIX step failed\\n");
        return 0;
    }

    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr, "C4-A.8 future low: TALTWT step failed\\n");
        return 0;
    }

    if (!check_word(
            &core,
            0x80000100u - 12u,
            T425_PROCESS_STATE_WAITING,
            "future low State"))
        return 0;

    if (!check_word(
            &core,
            0x80000100u - 20u,
            120u,
            "future low Time"))
        return 0;

    if (!check_word(
            &core,
            0x80000100u,
            UINT32_MAX,
            "future low DisableStatus"))
        return 0;

    if (!check_word(
            &core,
            T425_TPTR_LOC1,
            0x80000101u,
            "future low timer head"))
        return 0;

    /*
     * The instruction stream contains:
     *   PFIX 5  = 1 cycle
     *   TALTWT  = 48 cycles
     *
     * Total = 49 processor cycles.
     */
    if (core.cycles != 49u)
    {
        fprintf(stderr,
                "future low cycles: expected 49, got %llu\n",
                (unsigned long long)core.cycles);
        return 0;
    }

    /*
     * The process has no successor in the ready queues.
     */
    if (core.scheduler.current_wdesc != T425_NOT_PROCESS)
    {
        fprintf(stderr,
                "future low: expected no running process\n");
        return 0;
    }

    return 1;
}


static void test_taltwt_context_switch(void)
{
    t425_core_t core;
    uint32_t p1 = T425_INTERNAL_RAM_BASE + 0x100u;
    uint32_t p2 = T425_INTERNAL_RAM_BASE + 0x200u;
    uint32_t p1_iptr = T425_INTERNAL_RAM_BASE + 0x300u;
    uint32_t p2_iptr = T425_INTERNAL_RAM_BASE + 0x400u;
    uint32_t p1_wdesc = p1 | T425_PRIORITY_HIGH;
    uint32_t p2_wdesc = p2 | T425_PRIORITY_HIGH;

    t425_core_reset(&core);

    core.scheduler.current_wdesc = p1_wdesc;
    core.scheduler.running = 1u;
    core.cpu.Wptr = p1;
    core.cpu.Iptr = p1_iptr;

    assert(t425_memory_write_word(
        &core.memory, p1 - 4u, p1_iptr) == 0);
    assert(t425_memory_write_word(
        &core.memory, p1 - 8u, T425_NOT_PROCESS) == 0);
    assert(t425_memory_write_word(
        &core.memory, p1 - 12u,
        T425_PROCESS_STATE_ENABLING) == 0);
    assert(t425_memory_write_word(
        &core.memory, p1 - 16u,
        T425_TIMER_TIME_SET) == 0);
    assert(t425_memory_write_word(
        &core.memory, p1 - 20u, 120u) == 0);

    core.timer.high = 100u;

    core.cpu.A = 0x11111111u;
    core.cpu.B = 0x22222222u;
    core.cpu.C = 0x33333333u;

    /* TALTWT = PFIX 5 + OPR 1. */
    assert(t425_memory_write_byte(
        &core.memory, p1_iptr, 0x25u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, p1_iptr + 1u, 0xF1u) == 0);

    /* P2 is ready. */
    assert(t425_memory_write_word(
        &core.memory, p2 - 4u, p2_iptr) == 0);
    assert(t425_memory_write_word(
        &core.memory, p2 - 8u, T425_NOT_PROCESS) == 0);

    core.scheduler.high_head = p2_wdesc;
    core.scheduler.high_tail = p2_wdesc;

    /* Execute PFIX 5 and TALTWT. */
    assert(t425_core_step(&core) >= 0);
    assert(t425_core_step(&core) >= 0);

    assert(core.cpu.Wptr == p2);
    assert(core.cpu.Iptr == p2_iptr);

    /* Give P2 a distinctive context. */
    core.cpu.A = 0xAAAAAAAAu;
    core.cpu.B = 0xBBBBBBBBu;
    core.cpu.C = 0xCCCCCCCCu;

    /*
     * Explicitly requeue P1. This isolates context restoration
     * from the timer wake-up mechanism.
     */
    assert(t425_memory_write_word(
        &core.memory, p1 - 4u, p1_iptr) == 0);
    assert(t425_memory_write_word(
        &core.memory, p1 - 8u, T425_NOT_PROCESS) == 0);

    core.scheduler.high_head = p1_wdesc;
    core.scheduler.high_tail = p1_wdesc;

    /* STOPP = PFIX 1 + OPR 5. */
    assert(t425_memory_write_byte(
        &core.memory, p2_iptr, 0x21u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, p2_iptr + 1u, 0xF5u) == 0);

    core.cpu.Iptr = p2_iptr;

    assert(t425_core_step(&core) >= 0);
    assert(t425_core_step(&core) >= 0);

    /* P1 context must have been restored. */
    assert(core.cpu.Wptr == p1);
    assert(core.cpu.Iptr == p1_iptr);
    assert(core.cpu.A == 0x11111111u);
    assert(core.cpu.B == 0x22222222u);
    assert(core.cpu.C == 0x33333333u);
}

int main(void)
{
    if (!test_future_low())
        return 1;

    test_taltwt_context_switch();

    puts("C4-A.8 TALTWT future low: PASS");
    puts("C4-A.8 TALTWT context switch: PASS");
    puts("F08.1-C.4a.8 TALTWT tests: PASS");
    return 0;
}
