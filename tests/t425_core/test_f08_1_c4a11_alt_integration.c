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
    const uint32_t wdesc = wptr | T425_PRIORITY_LOW;

    (void)t425_memory_write_word(
        &core->memory, wptr - 4u, iptr);
    (void)t425_memory_write_word(
        &core->memory, wptr - 8u, T425_NOT_PROCESS);
    (void)t425_memory_write_word(
        &core->memory, wptr - 12u,
        T425_PROCESS_STATE_ENABLING);
    (void)t425_memory_write_word(
        &core->memory, wptr - 16u,
        T425_TIMER_TIME_NO_SET);
    (void)t425_memory_write_word(
        &core->memory, wptr - 20u, 0u);

    core->scheduler.current_wdesc = wdesc;
    core->scheduler.running = 1u;
    core->cpu.Wptr = wptr;
    core->cpu.Iptr = iptr;
}

static int execute_one(t425_core_t *core, uint8_t byte)
{
    (void)t425_memory_write_byte(
        &core->memory, core->cpu.Iptr, byte);

    return t425_core_step(core) >= 0;
}

static int execute_opr(
    t425_core_t *core,
    uint8_t pfix,
    uint8_t opr)
{
    (void)t425_memory_write_byte(
        &core->memory, core->cpu.Iptr, pfix);
    (void)t425_memory_write_byte(
        &core->memory, core->cpu.Iptr + 1u, opr);

    if (t425_core_step(core) < 0)
        return 0;

    if (t425_core_step(core) < 0)
        return 0;

    return 1;
}

static int test_talt_enbt_taltwt(void)
{
    t425_core_t core;
    const uint32_t wptr = 0x80000100u;
    const uint32_t iptr = 0x80000080u;
    const uint32_t wdesc = wptr | T425_PRIORITY_LOW;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    setup_process(&core, wptr, iptr);
    core.timer.low = 100u;

    /*
     * TALT = 0x4E.
     *
     * Establish the ALT descriptor in the workspace.
     */
    if (!execute_one(&core, 0x4Eu))
    {
        fprintf(stderr, "C4-A.11 TALT failed\n");
        return 0;
    }

    if (!check_word(
            &core, wptr - 12u,
            T425_PROCESS_STATE_ENABLING,
            "TALT State"))
        return 0;

    if (!check_word(
            &core, wptr - 16u,
            T425_TIMER_TIME_NO_SET,
            "TALT TLink"))
        return 0;

    /*
     * ENBT = OPR 0x47 = PFIX 4 + OPR 7.
     *
     * A != 0 enables the timer guard.
     * B supplies the requested deadline.
     */
    core.cpu.A = 1u;
    core.cpu.B = 120u;
    core.cpu.C = 777u;

    if (!execute_opr(&core, 0x24u, 0xF7u))
    {
        fprintf(stderr, "C4-A.11 ENBT failed\n");
        return 0;
    }

    if (!check_word(
            &core, wptr - 20u,
            120u,
            "ENBT Time"))
        return 0;

    if (!check_word(
            &core, wptr - 16u,
            T425_TIMER_TIME_SET,
            "ENBT TLink"))
        return 0;

    if (core.cpu.B != 777u || core.cpu.C != 0u)
    {
        fprintf(stderr,
                "ENBT registers: expected B=777 C=0, got B=%u C=%u\n",
                core.cpu.B,
                core.cpu.C);
        return 0;
    }

    /*
     * Restore the process operands used only by ENBT.
     * TALTWT itself does not depend on A/B/C here.
     */
    core.cpu.A = 0u;
    core.cpu.B = 0u;
    core.cpu.C = 0u;

    if (!execute_opr(&core, 0x25u, 0xF1u))
    {
        fprintf(stderr, "C4-A.11 TALTWT failed\n");
        return 0;
    }

    if (!check_word(
            &core, wptr - 12u,
            T425_PROCESS_STATE_WAITING,
            "TALTWT State"))
        return 0;

    if (!check_word(
            &core, wptr - 16u,
            T425_NOT_PROCESS,
            "TALTWT process TLink"))
        return 0;

    if (!check_word(
            &core, T425_TPTR_LOC1,
            wdesc,
            "TALTWT timer head"))
        return 0;

    if (core.scheduler.current_wdesc != T425_NOT_PROCESS)
    {
        fprintf(stderr,
                "TALTWT: process remained running unexpectedly\n");
        return 0;
    }

    puts("C4-A.11 TALT → ENBT → TALTWT integration: PASS");
    return 1;
}

static int test_talt_enbt_dist(void)
{
    t425_core_t core;
    const uint32_t wptr = 0x80000200u;
    const uint32_t iptr = 0x800000C0u;
    const uint32_t wdesc = wptr | T425_PRIORITY_LOW;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    setup_process(&core, wptr, iptr);
    core.timer.low = 100u;

    /*
     * TALT establishes the ALT descriptor.
     */
    if (!execute_one(&core, 0x4Eu))
    {
        fprintf(stderr, "C4-A.11 DIST: TALT failed\n");
        return 0;
    }

    /*
     * ENBT installs deadline 130.
     */
    core.cpu.A = 1u;
    core.cpu.B = 130u;
    core.cpu.C = 0u;

    if (!execute_opr(&core, 0x24u, 0xF7u))
    {
        fprintf(stderr, "C4-A.11 DIST: ENBT failed\n");
        return 0;
    }

    /*
     * TALTWT places the process on the timer queue.
     */
    core.cpu.A = 0u;
    core.cpu.B = 0u;
    core.cpu.C = 0u;

    if (!execute_opr(&core, 0x25u, 0xF1u))
    {
        fprintf(stderr, "C4-A.11 DIST: TALTWT failed\n");
        return 0;
    }

    if (!check_word(
            &core, T425_TPTR_LOC1,
            wdesc,
            "DIST initial timer head"))
        return 0;

    /*
     * DIST:
     *
     *   A = offset
     *   B = timer-selection flag
     *   C = requested timer value
     *
     * A nonzero B with a future C removes the active timer guard.
     */
    core.scheduler.current_wdesc = wdesc;
    core.scheduler.running = 1u;
    core.cpu.Wptr = wptr;
    core.cpu.A = 0u;
    core.cpu.B = 1u;
    core.cpu.C = 130u;

    if (!execute_opr(&core, 0x22u, 0xFEu))
    {
        fprintf(stderr, "C4-A.11 DIST failed\n");
        return 0;
    }

    if (!check_word(
            &core, T425_TPTR_LOC1,
            T425_NOT_PROCESS,
            "DIST timer head"))
        return 0;

    /*
     * DIST removes the process from the timer list. The TLink
     * therefore becomes the end-of-list value.
     */
    if (!check_word(
            &core, wptr - 16u,
            T425_NOT_PROCESS,
            "DIST TLink"))
        return 0;

    if (core.cpu.A != 0u ||
        core.cpu.B != 0u ||
        core.cpu.C != 0u)
    {
        fprintf(stderr,
                "DIST registers: expected A=0 B=0 C=0, got A=%u B=%u C=%u\n",
                core.cpu.A,
                core.cpu.B,
                core.cpu.C);
        return 0;
    }

    puts("C4-A.11 TALT → ENBT → DIST integration: PASS");
    return 1;
}

int main(void)
{
    if (!test_talt_enbt_taltwt())
        return 1;

    if (!test_talt_enbt_dist())
        return 1;

    puts("F08.1-C.4a.11 ALT integration tests: PASS");
    return 0;
}
