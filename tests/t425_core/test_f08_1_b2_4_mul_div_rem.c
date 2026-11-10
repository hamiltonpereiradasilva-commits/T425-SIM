#include <assert.h>
#include "f1_fixture_compat.h"
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

    code(core, address,
         (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu)));
    code(core, address + 1u,
         (uint8_t)(0xF0u | (opr & 0x0Fu)));

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(core->cpu.Oreg == ((uint32_t)opr & 0xF0u));

    return t425_core_step(core);
}

static void test_mul(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 7u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 6u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);

    assert(execute_opr(&core, 0x53u) == T425_STEP_OK);

    assert(core.cpu.A == 42u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 39u);
    assert(core.instructions == 2u);
}

static void test_div(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 3u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 20u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);

    assert(execute_opr(&core, 0x2Cu) == T425_STEP_OK);

    assert(core.cpu.A == 6u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 40u);
    assert(core.instructions == 2u);
}

static void test_rem(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 3u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 20u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);

    assert(execute_opr(&core, 0x1Fu) == T425_STEP_OK);

    assert(core.cpu.A == 2u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 38u);
    assert(core.instructions == 2u);
}

int main(void)
{
    test_mul();
    test_div();
    test_rem();

    puts("F08.1-B.2.4 MUL/DIV/REM tests: PASS");
    return 0;
}
