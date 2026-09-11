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

static void execute_opr(t425_core_t *core, uint8_t opr)
{
    /*
     * Construct the OPR operand through the real instruction stream.
     *
     * Example:
     *   OPR 0x46 -> 0x24 0xF6
     *   0x24 = PFIX 4
     *   0xF6 = OPR 6
     *
     * After PFIX:
     *   Oreg = 0x40
     *
     * After OPR decode:
     *   Oreg = 0x46
     */
    uint32_t address = core->cpu.Iptr;

    code(core, address,
         (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu)));
    code(core, address + 1u,
         (uint8_t)(0xF0u | (opr & 0x0Fu)));

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(core->cpu.Oreg == ((uint32_t)opr & 0xF0u));

    assert(t425_core_step(core) == T425_STEP_OK);

    /*
     * The completed OPR is consumed by finish(), which clears Oreg.
     */
    assert(core->cpu.Oreg == 0u);
}

static void test_and(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0xF0F0F0F0u;
    core.cpu.B = 0x0FF00FF0u;
    core.cpu.C = 0x12345678u;

    execute_opr(&core, 0x46u);

    assert(core.cpu.A == 0x00F000F0u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

static void test_or(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0xF000000Fu;
    core.cpu.B = 0x0F0000F0u;
    core.cpu.C = 0x87654321u;

    execute_opr(&core, 0x4Bu);

    assert(core.cpu.A == 0xFF0000FFu);
    assert(core.cpu.B == 0x87654321u);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

static void test_xor(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0xFFFF0000u;
    core.cpu.B = 0x0F0F0F0Fu;
    core.cpu.C = 0xCAFEBABEu;

    execute_opr(&core, 0x33u);

    assert(core.cpu.A == 0xF0F00F0Fu);
    assert(core.cpu.B == 0xCAFEBABEu);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

static void test_not(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0x0F0F0F0Fu;
    core.cpu.B = 0x12345678u;
    core.cpu.C = 0xCAFEBABEu;

    execute_opr(&core, 0x32u);

    assert(core.cpu.A == 0xF0F0F0F0u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.C == 0xCAFEBABEu);
    assert(core.cycles == 2u);
    assert(core.instructions == 2u);
}

int main(void)
{
    test_and();
    test_or();
    test_xor();
    test_not();

    puts("F08.1-B.2.1 AND/OR/XOR/NOT tests: PASS");
    return 0;
}
