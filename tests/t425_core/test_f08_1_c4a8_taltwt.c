#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "t425_core.h"
#include "t425_process.h"

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

int main(void)
{
    if (!test_future_low())
        return 1;

    puts("C4-A.8 TALTWT future low: PASS");
    puts("F08.1-C.4a.8 TALTWT tests: PASS");
    return 0;
}
