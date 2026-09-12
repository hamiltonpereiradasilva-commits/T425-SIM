#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void put_opr(t425_core_t *core, uint32_t opr)
{
    uint32_t address = T425_INTERNAL_RAM_BASE;

    core->cpu.Iptr = address;

    assert(t425_memory_write_byte(
        &core->memory, address,
        (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu))) == 0);

    assert(t425_memory_write_byte(
        &core->memory, address + 1u,
        (uint8_t)(0xF0u | (opr & 0x0Fu))) == 0);
}

static void exec_opr(t425_core_t *core, uint32_t opr)
{
    put_opr(core, opr);
    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static void test_in_range(void)
{
    t425_core_t c;

    t425_core_reset(&c);
    c.cpu.A = 20u;
    c.cpu.B = 7u;
    c.cpu.C = 123u;

    exec_opr(&c, 0x13u);

    assert(c.cpu.error == 0u);
    assert(c.cpu.A == 7u);
    assert(c.cpu.B == 123u);
    assert(c.cycles == 3u);
}

static void test_equal(void)
{
    t425_core_t c;

    t425_core_reset(&c);
    c.cpu.A = 20u;
    c.cpu.B = 20u;
    c.cpu.C = 123u;

    exec_opr(&c, 0x13u);

    assert(c.cpu.error == 0u);
    assert(c.cpu.A == 20u);
    assert(c.cpu.B == 123u);
    assert(c.cycles == 3u);
}

static void test_out_of_range(void)
{
    t425_core_t c;

    t425_core_reset(&c);
    c.cpu.A = 20u;
    c.cpu.B = 21u;
    c.cpu.C = 123u;

    exec_opr(&c, 0x13u);

    assert(c.cpu.error == 1u);
    assert(c.cpu.A == 21u);
    assert(c.cpu.B == 123u);
    assert(c.cycles == 3u);
}

int main(void)
{
    test_in_range();
    test_equal();
    test_out_of_range();

    puts("F08.1-C.3 CSUB0: PASS");
    return 0;
}
