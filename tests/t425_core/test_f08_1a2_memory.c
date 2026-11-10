#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void put_code(t425_core_t *core,
                     uint32_t address,
                     uint8_t byte)
{
    assert(t425_memory_write_byte(&core->memory, address, byte) == 0);
}

static void test_ldlp(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Wptr = 0x80000040u;
    core.cpu.A = 0x11111111u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x22222222u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x33333333u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Iptr = 0x80000000u;

    put_code(&core, 0x80000000u, 0x12u); /* LDLP #2 */

    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 0x80000048u);
    assert(core.cpu.B == 0x11111111u);
    assert(core.cpu.C == 0x22222222u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 1u);
}

static void test_ldnlp(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.A = 0x80000040u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x11111111u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x22222222u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Iptr = 0x80000000u;

    put_code(&core, 0x80000000u, 0x52u); /* LDNLP #2 */

    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 0x80000048u);
    assert(core.cpu.B == 0x11111111u);
    assert(core.cpu.C == 0x22222222u);
    assert(core.cpu.Oreg == 0u);
}

static void test_ldl(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Wptr = 0x80000040u;
    core.cpu.A = 0xAAAAAAAAu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0xBBBBBBBBu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0xCCCCCCCCu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Iptr = 0x80000000u;

    assert(t425_memory_write_word(
        &core.memory, 0x80000048u, 0x12345678u) == 0);

    put_code(&core, 0x80000000u, 0x72u); /* LDL #2 */

    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 0x12345678u);
    assert(core.cpu.B == 0xAAAAAAAAu);
    assert(core.cpu.C == 0xBBBBBBBBu);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 2u);
}

static void test_ldnl(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.A = 0x80000040u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0xAAAAAAAAu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0xBBBBBBBBu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Iptr = 0x80000000u;

    assert(t425_memory_write_word(
        &core.memory, 0x80000048u, 0x87654321u) == 0);

    put_code(&core, 0x80000000u, 0x32u); /* LDNL #2 */

    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 0x87654321u);
    assert(core.cpu.B == 0xAAAAAAAAu);
    assert(core.cpu.C == 0xBBBBBBBBu);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 2u);
}

static void test_adc(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.A = 100u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0xAAAAAAAAu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0xBBBBBBBBu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.Iptr = 0x80000000u;

    put_code(&core, 0x80000000u, 0x85u); /* ADC #5 */

    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 105u);
    assert(core.cpu.B == 0xAAAAAAAAu);
    assert(core.cpu.C == 0xBBBBBBBBu);
    assert(core.cpu.Oreg == 0u);
    assert(core.cpu.error == 0u);
}

int main(void)
{
    test_ldlp();
    test_ldnlp();
    test_ldl();
    test_ldnl();
    test_adc();

    puts("F08.1-A.2 memory/workspace: PASS");
    return 0;
}
