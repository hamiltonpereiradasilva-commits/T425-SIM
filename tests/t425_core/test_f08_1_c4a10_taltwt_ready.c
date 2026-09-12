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
    uint32_t state,
    uint32_t tlink,
    uint32_t time)
{
    const uint32_t wdesc = wptr | T425_PRIORITY_LOW;

    (void)t425_memory_write_word(
        &core->memory, wptr - 4u, iptr);
    (void)t425_memory_write_word(
        &core->memory, wptr - 8u, T425_NOT_PROCESS);
    (void)t425_memory_write_word(
        &core->memory, wptr - 12u, state);
    (void)t425_memory_write_word(
        &core->memory, wptr - 16u, tlink);
    (void)t425_memory_write_word(
        &core->memory, wptr - 20u, time);

    core->scheduler.current_wdesc = wdesc;
    core->scheduler.running = 1u;
    core->cpu.Wptr = wptr;
    core->cpu.Iptr = iptr;
}

static int execute_taltwt(t425_core_t *core)
{
    (void)t425_memory_write_byte(
        &core->memory, core->cpu.Iptr, 0x25u);
    (void)t425_memory_write_byte(
        &core->memory, core->cpu.Iptr + 1u, 0xF1u);

    if (t425_core_step(core) < 0)
    {
        fprintf(stderr, "PFIX step failed\n");
        return 0;
    }

    if (t425_core_step(core) < 0)
    {
        fprintf(stderr, "TALTWT step failed\n");
        return 0;
    }

    return 1;
}

static int test_ready_state(void)
{
    t425_core_t core;
    const uint32_t wptr = 0x80000100u;
    const uint32_t iptr = 0x80000080u;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    core.timer.low = 100u;
    core.timer.running = 1u;

    /*
     * TALTWT with an already-ready ALT.
     *
     * Architectural definition:
     *   Time := current timer value
     *   State remains Ready
     *   process is not descheduled
     */
    setup_process(
        &core,
        wptr,
        iptr,
        T425_PROCESS_STATE_READY,
        T425_TIMER_TIME_SET,
        50u);

    if (!execute_taltwt(&core))
    {
        fprintf(stderr, "C4-A.10 ready: execution failed\n");
        return 0;
    }

    if (!check_word(
            &core,
            wptr - 12u,
            T425_PROCESS_STATE_READY,
            "ready State"))
        return 0;

    if (!check_word(
            &core,
            wptr - 20u,
            100u,
            "ready Time"))
        return 0;

    if (core.scheduler.current_wdesc !=
        (wptr | T425_PRIORITY_LOW))
    {
        fprintf(stderr,
                "ready: process was unexpectedly descheduled\n");
        return 0;
    }

    if (core.timer.low != 100u)
    {
        fprintf(stderr,
                "ready timer changed unexpectedly: got %u\n",
                core.timer.low);
        return 0;
    }

    /*
     * PFIX = 1 cycle.
     * TALTWT with an already-ready ALT = 15 cycles.
     * Total = 16 cycles.
     */
    if (core.cycles != 16u)
    {
        fprintf(stderr,
                "ready cycles: expected 16, got %llu\n",
                (unsigned long long)core.cycles);
        return 0;
    }

    puts("C4-A.10 TALTWT already-ready guard: PASS");
    return 1;
}

static int test_expired_timer_guard(void)
{
    t425_core_t core;
    const uint32_t wptr = 0x80000100u;
    const uint32_t iptr = 0x80000080u;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    /*
     * Timer guard is already expired:
     *
     *   current timer = 100
     *   requested time = 90
     *
     * The process must NOT be descheduled.
     * TALTWT must make State = Ready and record the
     * current timer value in Time.
     */
    core.timer.low = 100u;

    setup_process(
        &core,
        wptr,
        iptr,
        T425_PROCESS_STATE_ENABLING,
        T425_TIMER_TIME_SET,
        90u);

    if (!execute_taltwt(&core))
    {
        fprintf(stderr, "C4-A.10 expired: execution failed\n");
        return 0;
    }

    if (!check_word(
            &core,
            wptr - 12u,
            T425_PROCESS_STATE_READY,
            "expired State"))
        return 0;

    if (!check_word(
            &core,
            wptr - 20u,
            100u,
            "expired Time"))
        return 0;

    if (core.scheduler.current_wdesc !=
        (wptr | T425_PRIORITY_LOW))
    {
        fprintf(stderr,
                "expired: process was unexpectedly descheduled\n");
        return 0;
    }

    if (core.scheduler.low_head != T425_NOT_PROCESS ||
        core.scheduler.low_tail != T425_NOT_PROCESS)
    {
        fprintf(stderr,
                "expired: process was unexpectedly inserted in ready queue\n");
        return 0;
    }

    /*
     * PFIX = 1 cycle.
     * TALTWT with an already-expired timer guard = 15 cycles.
     * Total = 16 cycles.
     */
    if (core.cycles != 16u)
    {
        fprintf(stderr,
                "expired cycles: expected 16, got %llu\n",
                (unsigned long long)core.cycles);
        return 0;
    }

    puts("C4-A.10 TALTWT expired timer guard: PASS");
    return 1;
}

int main(void)
{
    if (!test_ready_state())
        return 1;

    if (!test_expired_timer_guard())
        return 1;

    puts("F08.1-C.4a.10 TALTWT ready/expired tests: PASS");
    return 0;
}
