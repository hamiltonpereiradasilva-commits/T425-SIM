#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void code(t425_core_t *core, uint32_t address, uint8_t byte)
{
    assert(t425_memory_write_byte(&core->memory, address, byte) == 0);
}

static int execute_opr(t425_core_t *core, uint8_t opr)
{
    uint32_t address = core->cpu.Iptr;

    code(core, address,
         (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu)));
    code(core, address + 1u,
         (uint8_t)(0xF0u | (opr & 0x0Fu)));

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(core->cpu.Oreg == ((uint32_t)opr & 0xF0u));

    return t425_core_step(core);
}

static void test_div_zero(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0u;
    core.cpu.B = 20u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x2Cu) == T425_STEP_OK);
    assert(core.cpu.error == 1u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 40u);
    assert(core.instructions == 2u);
}

static void test_div_minint_minus_one(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0xFFFFFFFFu; /* -1 */
    core.cpu.B = 0x80000000u; /* INT32_MIN */
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x2Cu) == T425_STEP_OK);
    assert(core.cpu.error == 1u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 40u);
    assert(core.instructions == 2u);
}

static void test_rem_zero(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0u;
    core.cpu.B = 20u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x1Fu) == T425_STEP_OK);
    assert(core.cpu.error == 1u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 38u);
    assert(core.instructions == 2u);
}

static void test_rem_minint_minus_one(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0xFFFFFFFFu; /* -1 */
    core.cpu.B = 0x80000000u; /* INT32_MIN */
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x1Fu) == T425_STEP_OK);
    assert(core.cpu.error == 1u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 38u);
    assert(core.instructions == 2u);
}

int main(void)
{
    test_div_zero();
    test_div_minint_minus_one();
    test_rem_zero();
    test_rem_minint_minus_one();

    puts("F08.1-B.2.4 DIV/REM error tests: PASS");
    return 0;
}
