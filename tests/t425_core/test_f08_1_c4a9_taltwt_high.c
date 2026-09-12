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
    uint32_t iptr)
{
    const uint32_t wdesc = wptr | T425_PRIORITY_HIGH;

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
        &core->memory, wptr - 20u, 10u);

    core->scheduler.current_wdesc = wdesc;
    core->scheduler.running = 1u;
    core->cpu.Wptr = wptr;
    core->cpu.Iptr = iptr;
}

int main(void)
{
    t425_core_t core;
    const uint32_t wptr = 0x80000100u;
    const uint32_t iptr = 0x80000080u;
    const uint32_t wdesc = wptr | T425_PRIORITY_HIGH;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    setup_process(&core, wptr, iptr);

    /*
     * HIGH timer:
     *   20 processor cycles per timer tick.
     *
     * Start at 8 and wait for time 10.
     */
    core.timer.high = 8u;
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

    /*
     * PFIX.
     */
    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr,
                "C4-A.9 high: PFIX step failed\n");
        return 1;
    }

    /*
     * TALTWT.
     *
     * It consumes 48 cycles. The HIGH timer therefore advances
     * by 2 ticks:
     *
     *   8 -> 10
     *
     * The target is reached during TALTWT's execution interval.
     */
    if (t425_core_step(&core) < 0)
    {
        fprintf(stderr,
                "C4-A.9 high: TALTWT step failed\n");
        return 1;
    }

    if (t425_timer_read_high(&core.timer) != 10u)
    {
        fprintf(stderr,
                "high timer: expected 10, got %u\n",
                t425_timer_read_high(&core.timer));
        return 1;
    }

    /*
     * The timer service is invoked by finish(). Therefore the
     * process should already have been awakened by the end of
     * TALTWT if the architectural timing path is implemented
     * consistently.
     */
    if (!check_word(
            &core,
            wptr - 12u,
            T425_PROCESS_STATE_READY,
            "high ready State"))
        return 1;

    if (!check_word(
            &core,
            wptr - 16u,
            T425_TIMER_TIME_SET,
            "high TLink"))
        return 1;

    if (!check_word(
            &core,
            T425_TPTR_LOC0,
            T425_NOT_PROCESS,
            "high timer head"))
        return 1;

    if (core.scheduler.high_head != wdesc)
    {
        fprintf(stderr,
                "high ready head: expected 0x%08x, got 0x%08x\n",
                wdesc,
                core.scheduler.high_head);
        return 1;
    }

    if (core.scheduler.high_tail != wdesc)
    {
        fprintf(stderr,
                "high ready tail: expected 0x%08x, got 0x%08x\n",
                wdesc,
                core.scheduler.high_tail);
        return 1;
    }

    if (core.scheduler.current_wdesc != T425_NOT_PROCESS)
    {
        fprintf(stderr,
                "high: expected no running process after timer service\n");
        return 1;
    }

    if (core.cycles != 49u)
    {
        fprintf(stderr,
                "high cycles: expected 49, got %llu\n",
                (unsigned long long)core.cycles);
        return 1;
    }

    puts("C4-A.9 TALTWT future high / expiry: PASS");
    puts("F08.1-C.4a.9 TALTWT high tests: PASS");

    return 0;
}
