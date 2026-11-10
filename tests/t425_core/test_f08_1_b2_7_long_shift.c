#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>
#include "t425_core.h"

#define RAM T425_INTERNAL_RAM_BASE

static void execute_opr(t425_core_t *core, uint8_t opr)
{
    core->memory.data[0] = (uint8_t)(0x20u | (opr >> 4));
    core->memory.data[1] = (uint8_t)(0xF0u | (opr & 0x0Fu));
    core->cpu.Iptr = RAM;

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static uint64_t result64(const t425_core_t *core)
{
    return ((uint64_t)core->cpu.B << 32) | core->cpu.A;
}

static void test_lshl_n0(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 0u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0x0123456789ABCDEF));
    assert(core.cycles == 4u);
    assert(core.instructions == 2u);
}

static void test_lshl_n1(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 1u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0x02468ACF13579BDE));
    assert(core.cycles == 5u);
}

static void test_lshl_n31(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 31u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0xC4D5E6F780000000));
    assert(core.cycles == 35u);
}

static void test_lshl_n32(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 32u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0x89ABCDEF00000000));
    assert(core.cycles == 5u);
}

static void test_lshl_n33(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 33u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0x13579BDE00000000));
    assert(core.cycles == 6u);
}

static void test_lshl_n63(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 63u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0x8000000000000000));
    assert(core.cycles == 36u);
}

static void test_lshl_n64(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 64u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x01234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x36u);

    assert(result64(&core) == UINT64_C(0));
    assert(core.cycles == 37u);
}

static void test_lshr_n0(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 0u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(0x8123456789ABCDEF));
    assert(core.cycles == 4u);
}

static void test_lshr_n1(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 1u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(0x4091A2B3C4D5E6F7));
    assert(core.cycles == 5u);
}

static void test_lshr_n31(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 31u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(0x0000000102468ACF));
    assert(core.cycles == 35u);
}

static void test_lshr_n32(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 32u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(0x81234567));
    assert(core.cycles == 5u);
}

static void test_lshr_n33(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 33u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(0x4091A2B3));
    assert(core.cycles == 6u);
}

static void test_lshr_n63(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 63u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(1));
    assert(core.cycles == 36u);
}

static void test_lshr_n64(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    core.cpu.A = 64u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x89ABCDEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x81234567u; t425_fixture_sync_slaves(&core.cpu);

    execute_opr(&core, 0x35u);

    assert(result64(&core) == UINT64_C(0));
    assert(core.cycles == 37u);
}

int main(void)
{
    test_lshl_n0();
    test_lshl_n1();
    test_lshl_n31();
    test_lshl_n32();
    test_lshl_n33();
    test_lshl_n63();
    test_lshl_n64();

    test_lshr_n0();
    test_lshr_n1();
    test_lshr_n31();
    test_lshr_n32();
    test_lshr_n33();
    test_lshr_n63();
    test_lshr_n64();

    puts("F08.1-B.2.7 LSHL/LSHR tests: PASS");
    return 0;
}
