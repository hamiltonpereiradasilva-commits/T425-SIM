#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#include "t425_core.h"

static int failures;

static void check_u32(const char *name, uint32_t got, uint32_t expected)
{
    if (got != expected)
    {
        printf("FAIL: %s got=0x%08x expected=0x%08x\n",
               name, got, expected);
        ++failures;
    }
}

static void check_i(const char *name, int got, int expected)
{
    if (got != expected)
    {
        printf("FAIL: %s got=%d expected=%d\n",
               name, got, expected);
        ++failures;
    }
}

static void reset_core(t425_core_t *core)
{
    t425_core_reset(core);
    core->cpu.Iptr = T425_INTERNAL_RAM_BASE;
}

static int run_opr(t425_core_t *core, uint32_t op)
{
    uint32_t ip = core->cpu.Iptr;
    uint8_t pfix = (uint8_t)(0x20u | ((op >> 4u) & 0x0Fu));
    uint8_t opr = (uint8_t)(0xF0u | (op & 0x0Fu));

    if (op > 0xFFu)
    {
        (void)t425_memory_write_byte(&core->memory,
                                     ip, 0x21u);
        (void)t425_memory_write_byte(&core->memory,
                                     ip + 1u, 0x27u);
        (void)t425_memory_write_byte(&core->memory,
                                     ip + 2u, 0xFCu);

        for (int i = 0; i < 3; ++i)
        {
            int rc = t425_core_step(core);
            if (rc != T425_STEP_OK)
                return rc;
        }

        return T425_STEP_OK;
    }

    (void)t425_memory_write_byte(&core->memory, ip, pfix);
    (void)t425_memory_write_byte(&core->memory, ip + 1u, opr);

    if (t425_core_step(core) != T425_STEP_OK)
        return T425_STEP_UNIMPL;

    return t425_core_step(core);
}

int main(void)
{
    t425_core_t core;

    /* GCALL */
    reset_core(&core);
    core.cpu.A = T425_INTERNAL_RAM_BASE + 100u;

    check_i("GCALL", run_opr(&core, 0x06u), T425_STEP_OK);
    check_u32("GCALL A", core.cpu.A,
              T425_INTERNAL_RAM_BASE + 2u);
    check_u32("GCALL Iptr", core.cpu.Iptr,
              T425_INTERNAL_RAM_BASE + 100u);

    /* LDPI */
    reset_core(&core);
    core.cpu.A = 7u;

    check_i("LDPI", run_opr(&core, 0x1Bu), T425_STEP_OK);
    check_u32("LDPI A", core.cpu.A,
              T425_INTERNAL_RAM_BASE + 9u);

    /* TESTPRANAL */
    reset_core(&core);
    core.cpu.A = 0x11111111u;
    core.cpu.B = 0x22222222u;
    core.cpu.analyse = 0u;

    check_i("TESTPRANAL",
            run_opr(&core, 0x2Au),
            T425_STEP_OK);
    check_u32("TESTPRANAL A", core.cpu.A, 0u);
    check_u32("TESTPRANAL B", core.cpu.B, 0x11111111u);
    check_u32("TESTPRANAL C", core.cpu.C, 0x22222222u);

    /* SAVEH */
    reset_core(&core);
    core.scheduler.high_head = 0x11111111u;
    core.scheduler.high_tail = 0x22222222u;
    core.cpu.A = T425_INTERNAL_RAM_BASE + 100u;
    core.cpu.B = 0xAAAAAAAAu;
    core.cpu.C = 0xBBBBBBBBu;

    check_i("SAVEH", run_opr(&core, 0x3Eu), T425_STEP_OK);

    uint32_t v1 = 0;
    uint32_t v2 = 0;

    (void)t425_memory_read_word(&core.memory,
                                T425_INTERNAL_RAM_BASE + 100u,
                                &v1);
    (void)t425_memory_read_word(&core.memory,
                                T425_INTERNAL_RAM_BASE + 104u,
                                &v2);

    check_u32("SAVEH front", v1, 0x11111111u);
    check_u32("SAVEH back", v2, 0x22222222u);
    check_u32("SAVEH A", core.cpu.A, 0xAAAAAAAAu);
    check_u32("SAVEH B", core.cpu.B, 0xBBBBBBBBu);

    /* SAVEL */
    reset_core(&core);
    core.scheduler.low_head = 0x33333333u;
    core.scheduler.low_tail = 0x44444444u;
    core.cpu.A = T425_INTERNAL_RAM_BASE + 120u;
    core.cpu.B = 0xCCCCCCCCu;
    core.cpu.C = 0xDDDDDDDDu;

    check_i("SAVEL", run_opr(&core, 0x3Du), T425_STEP_OK);

    v1 = 0;
    v2 = 0;

    (void)t425_memory_read_word(&core.memory,
                                T425_INTERNAL_RAM_BASE + 120u,
                                &v1);
    (void)t425_memory_read_word(&core.memory,
                                T425_INTERNAL_RAM_BASE + 124u,
                                &v2);

    check_u32("SAVEL front", v1, 0x33333333u);
    check_u32("SAVEL back", v2, 0x44444444u);
    check_u32("SAVEL A", core.cpu.A, 0xCCCCCCCCu);
    check_u32("SAVEL B", core.cpu.B, 0xDDDDDDDDu);

    /* FMUL: 0.5 * 0.5 ~= 0.25 */
    reset_core(&core);
    core.cpu.A = 0x40000000u;
    core.cpu.B = 0x40000000u;
    core.cpu.C = 0x12345678u;

    check_i("FMUL", run_opr(&core, 0x72u), T425_STEP_OK);
    check_u32("FMUL A", core.cpu.A, 0x20000000u);
    check_u32("FMUL B", core.cpu.B, 0x12345678u);

    /* FMUL overflow */
    reset_core(&core);
    core.cpu.A = (uint32_t)INT32_MIN;
    core.cpu.B = (uint32_t)INT32_MIN;

    check_i("FMUL overflow",
            run_opr(&core, 0x72u),
            T425_STEP_OK);
    check_u32("FMUL error", core.cpu.error, 1u);

    /* Timers */
    reset_core(&core);
    core.timer.interrupt_high_enabled = 1u;
    core.timer.interrupt_low_enabled = 1u;

    check_i("TIMERDISABLEH",
            run_opr(&core, 0x7Au),
            T425_STEP_OK);
    check_u32("TIMERDISABLEH state",
              core.timer.interrupt_high_enabled, 0u);

    check_i("TIMERDISABLEL",
            run_opr(&core, 0x7Bu),
            T425_STEP_OK);
    check_u32("TIMERDISABLEL state",
              core.timer.interrupt_low_enabled, 0u);

    check_i("TIMERENABLEH",
            run_opr(&core, 0x7Cu),
            T425_STEP_OK);
    check_u32("TIMERENABLEH state",
              core.timer.interrupt_high_enabled, 1u);

    check_i("TIMERENABLEL",
            run_opr(&core, 0x7Du),
            T425_STEP_OK);
    check_u32("TIMERENABLEL state",
              core.timer.interrupt_low_enabled, 1u);

    /* LDMEMSTARTVAL */
    reset_core(&core);
    core.cpu.A = 0x11111111u;
    core.cpu.B = 0x22222222u;
    core.cpu.C = 0x33333333u;

    check_i("LDMEMSTARTVAL",
            run_opr(&core, 0x7Eu),
            T425_STEP_OK);

    check_u32("LDMEMSTARTVAL A",
              core.cpu.A,
              T425_INTERNAL_RAM_BASE + 0x70u);
    check_u32("LDMEMSTARTVAL B",
              core.cpu.B,
              0x11111111u);
    check_u32("LDMEMSTARTVAL C",
              core.cpu.C,
              0x22222222u);

    /* LDDEVID */
    reset_core(&core);
    core.cpu.devid = 5u;

    check_i("LDDEVID",
            run_opr(&core, 0x17Cu),
            T425_STEP_OK);
    check_u32("LDDEVID A", core.cpu.A, 5u);

    if (failures)
    {
        printf("F08.1-C.6.3 Phase 1: FAIL (%d failures)\n",
               failures);
        return 1;
    }

    printf("F08.1-C.6.3 Phase 1: PASS\n");
    return 0;
}
