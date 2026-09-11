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
     *   OPR 0x41 -> 0x24 0xF1
     *   0x24 = PFIX 4
     *   0xF1 = OPR 1
     */
    code(core, address,
         (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu)));
    code(core, address + 1u,
         (uint8_t)(0xF0u | (opr & 0x0Fu)));

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(core->cpu.Oreg == ((uint32_t)opr & 0xF0u));

    return t425_core_step(core);
}

static void test_shl(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 4u;
    core.cpu.B = 0x00000003u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x41u) == T425_STEP_OK);

    assert(core.cpu.A == 0x00000030u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 7u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_shr(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 4u;
    core.cpu.B = 0x80000030u;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x40u) == T425_STEP_OK);

    assert(core.cpu.A == 0x08000003u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 7u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_shl_zero(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0u;
    core.cpu.B = 0x89ABCDEFu;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x41u) == T425_STEP_OK);

    assert(core.cpu.A == 0x89ABCDEFu);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 3u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_shr_zero(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0u;
    core.cpu.B = 0x89ABCDEFu;
    core.cpu.C = 0x12345678u;

    assert(execute_opr(&core, 0x40u) == T425_STEP_OK);

    assert(core.cpu.A == 0x89ABCDEFu);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 3u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

static void test_shr_logical_fill(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 1u;
    core.cpu.B = 0x80000000u;
    core.cpu.C = 0xCAFEBABEu;

    assert(execute_opr(&core, 0x40u) == T425_STEP_OK);

    assert(core.cpu.A == 0x40000000u);
    assert(core.cpu.B == 0xCAFEBABEu);
    assert(core.cycles == 4u);
    assert(core.instructions == 2u);
    assert(core.cpu.Oreg == 0u);
}

int main(void)
{
    test_shl();
    test_shr();
    test_shl_zero();
    test_shr_zero();
    test_shr_logical_fill();

    puts("F08.1-B.2.3 SHL/SHR tests: PASS");
    return 0;
}
