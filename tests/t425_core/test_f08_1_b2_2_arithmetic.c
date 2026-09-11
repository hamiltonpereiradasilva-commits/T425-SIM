#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void code(t425_core_t *core, uint32_t address, uint8_t byte)
{
    assert(t425_memory_write_byte(
        &core->memory,
        address,
        byte) == 0);
}

static int execute_opr(t425_core_t *core, uint8_t opr)
{
    uint32_t address = core->cpu.Iptr;

    /*
     * Construct the OPR operand through the real instruction stream.
     *
     * Example:
     *   OPR 0x05 -> 0x20 0xF5
     *   0x20 = PFIX 0
     *   0xF5 = OPR 5
     */
    code(core, address,
         (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu)));
    code(core, address + 1u,
         (uint8_t)(0xF0u | (opr & 0x0Fu)));

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(core->cpu.Oreg == ((uint32_t)opr & 0xF0u));

    return t425_core_step(core);
}

static void test_add(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 10u;
    core.cpu.B = 7u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x05u) == T425_STEP_OK);

    assert(core.cpu.A == 17u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_sub(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 3u;
    core.cpu.B = 10u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x0Cu) == T425_STEP_OK);

    /*
     * SUB is B - A.
     */
    assert(core.cpu.A == 7u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_diff(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 3u;
    core.cpu.B = 10u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x04u) == T425_STEP_OK);

    /*
     * DIFF is unsigned B - A.
     */
    assert(core.cpu.A == 7u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_sum(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 10u;
    core.cpu.B = 7u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x52u) == T425_STEP_OK);

    /*
     * SUM is unchecked B + A.
     */
    assert(core.cpu.A == 17u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_gt(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 3u;
    core.cpu.B = 10u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x09u) == T425_STEP_OK);

    /*
     * GT: A = 1 when B > A, otherwise A = 0.
     */
    assert(core.cpu.A == 1u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_gt_false(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 10u;
    core.cpu.B = 3u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x09u) == T425_STEP_OK);

    assert(core.cpu.A == 0u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_add_overflow_positive(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    core.cpu.A = 0x7FFFFFFFu;
    core.cpu.B = 1u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x05u) == T425_STEP_OK);

    assert(core.cpu.A == 0x80000000u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.error == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

static void test_add_overflow_negative(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    core.cpu.A = 0x80000000u;
    core.cpu.B = 0xFFFFFFFFu;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x05u) == T425_STEP_OK);

    assert(core.cpu.A == 0x7FFFFFFFu);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.error == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

static void test_sub_overflow_positive(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    core.cpu.A = 0xFFFFFFFFu;
    core.cpu.B = 0x7FFFFFFFu;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x0Cu) == T425_STEP_OK);

    assert(core.cpu.A == 0x80000000u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.error == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

static void test_sub_overflow_negative(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    core.cpu.A = 1u;
    core.cpu.B = 0x80000000u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x0Cu) == T425_STEP_OK);

    assert(core.cpu.A == 0x7FFFFFFFu);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.error == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

int main(void)
{
    test_add();
    test_sub();
    test_diff();
    test_sum();
    test_gt();
    test_gt_false();

    test_add_overflow_positive();
    test_add_overflow_negative();
    test_sub_overflow_positive();
    test_sub_overflow_negative();

    puts("F08.1-B.2.2 ADD/SUB/DIFF/SUM/GT/OVERFLOW tests: PASS");
    return 0;
}
