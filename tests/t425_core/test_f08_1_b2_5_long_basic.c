#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

#define RAM 0x80000000u

static void execute_opr(t425_core_t *core, uint8_t opr)
{
    core->memory.data[0] = (uint8_t)(0x20u | (opr >> 4));
    core->memory.data[1] = (uint8_t)(0xF0u | (opr & 0x0Fu));

    core->cpu.Iptr = RAM;

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static void test_prod(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 7u;
    core.cpu.B = 6u;
    core.cpu.C = 0x12345678u;

    execute_opr(&core, 0x08u);



    assert(core.cpu.A == 42u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.C == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 7u);
    assert(core.instructions == 2u);
}

static void test_ladd(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 10u;
    core.cpu.B = 20u;
    core.cpu.C = 0x12345679u;

    execute_opr(&core, 0x16u);

    assert(core.cpu.A == 31u);
    assert(core.cpu.B == 0x12345679u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cpu.error == 0u);
    assert(core.cycles == 3u);
    assert(core.instructions == 2u);
}

static void test_lsub(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 10u;
    core.cpu.B = 20u;
    core.cpu.C = 0x12345678u;

    execute_opr(&core, 0x38u);

    assert(core.cpu.A == 10u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cpu.error == 0u);
    assert(core.cycles == 3u);
    assert(core.instructions == 2u);
}

static void test_lsum(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 0xFFFFFFFFu;
    core.cpu.B = 1u;
    core.cpu.C = 0x12345679u;

    execute_opr(&core, 0x37u);

    assert(core.cpu.A == 1u);
    assert(core.cpu.B == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 4u);
    assert(core.instructions == 2u);
}

static void test_ldiff(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 20u;
    core.cpu.B = 10u;
    core.cpu.C = 0x12345679u;

    execute_opr(&core, 0x4Fu);

    assert(core.cpu.A == 0xFFFFFFF5u);
    assert(core.cpu.B == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cpu.error == 0u);
    assert(core.cycles == 4u);
    assert(core.instructions == 2u);
}

int main(void)
{
    test_prod();
    test_ladd();
    test_lsub();
    test_lsum();
    test_ldiff();

    puts("F08.1-B.2.5 PROD/LADD/LSUB/LSUM/LDIFF tests: PASS");
    return 0;
}
