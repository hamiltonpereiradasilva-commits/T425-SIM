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

static int test_taltwt_wakeup_low(void)
{
    t425_core_t core;
    const uint32_t wptr = 0x80000100u;
    const uint32_t iptr = 0x80000080u;
    const uint32_t wdesc = wptr | T425_PRIORITY_LOW;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    setup_process(
        &core,
        wptr,
        iptr,
        T425_PRIORITY_LOW);

    /*
     * Low timer is currently 119 and ALT time is 120.
     *
     * TALTWT consumes 48 processor cycles. The low timer tick
     * is 1280 processor cycles, so it must not expire during
     * TALTWT itself.
     */
    core.timer.low = 119u;
    core.timer.running = 1u;

    /*
     * TALTWT = OPR 0x51:
     *   PFIX 5 = 0x25
     *   OPR 1  = 0xF1
     */
    (void)t425_memory_write_byte(
        &core.memory, iptr, 0x25u);
    (void)t425_memory_write_byte(
        &core.memory, iptr + 1u, 0xF1u);

    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr,
                "C4-A.8 wakeup low: PFIX step failed\n");
        return 0;
    }

    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr,
                "C4-A.8 wakeup low: TALTWT step failed\n");
        return 0;
    }

    if (!check_word(
            &core,
            wptr - 12u,
            T425_PROCESS_STATE_WAITING,
            "wakeup low waiting State"))
        return 0;

    if (!check_word(
            &core,
            T425_TPTR_LOC1,
            wdesc,
            "wakeup low timer head"))
        return 0;

    /*
     * TALTWT consumed 48 cycles. The low timer therefore has
     * 48 cycles accumulated toward its next 1280-cycle tick.
     *
     * Add the remaining 1232 cycles so the low timer advances
     * exactly one tick: 119 -> 120.
     */
    t425_timer_advance(&core.timer, 1232u);

    if (t425_timer_read_low(&core.timer) != 120u)
    {
        fprintf(stderr,
                "wakeup low timer: expected 120, got %u\n",
                t425_timer_read_low(&core.timer));
        return 0;
    }

    /*
     * Put TALT at the current Iptr. It costs 4 cycles and its
     * finish() invokes timer_service(), which must wake the
     * expired TALTWT process.
     */
    (void)t425_memory_write_byte(
        &core.memory,
        core.cpu.Iptr,
        0x4Eu);

    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr,
                "C4-A.8 wakeup low: TALT service step failed\n");
        return 0;
    }

    if (!check_word(
            &core,
            wptr - 12u,
            T425_PROCESS_STATE_READY,
            "wakeup low ready State"))
        return 0;

    if (!check_word(
            &core,
            wptr - 16u,
            T425_TIMER_TIME_SET,
            "wakeup low TLink"))
        return 0;

    if (!check_word(
            &core,
            T425_TPTR_LOC1,
            T425_NOT_PROCESS,
            "wakeup low timer head"))
        return 0;

    if (core.scheduler.low_head != wdesc)
    {
        fprintf(stderr,
                "wakeup low ready head: expected 0x%08x, got 0x%08x\n",
                wdesc,
                core.scheduler.low_head);
        return 0;
    }

    if (core.scheduler.low_tail != wdesc)
    {
        fprintf(stderr,
                "wakeup low ready tail: expected 0x%08x, got 0x%08x\n",
                wdesc,
                core.scheduler.low_tail);
        return 0;
    }

    if (core.scheduler.current_wdesc != T425_NOT_PROCESS)
    {
        fprintf(stderr,
                "wakeup low: expected no running process after timer service\n");
        return 0;
    }

    puts("C4-A.8 TALTWT wakeup low: PASS");
    return 1;
}

int main(void)
{
    if (!test_taltwt_wakeup_low())
        return 1;

    puts("F08.1-C.4a.8 TALTWT wakeup tests: PASS");
    return 0;
}
