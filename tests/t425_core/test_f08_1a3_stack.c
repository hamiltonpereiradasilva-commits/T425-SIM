#include <assert.h>
#include <stdio.h>
#include "t425_core.h"

static void code(t425_core_t *c, uint8_t b)
{
    assert(t425_memory_write_byte(&c->memory, 0x80000000u, b) == 0);
    c->cpu.Iptr = 0x80000000u;
}

static void test_ajw(void)
{
    t425_core_t c;

    t425_core_reset(&c);
    c.cpu.Wptr = 0x80000040u;
    code(&c, 0xB2u);

    assert(t425_core_step(&c) == T425_STEP_OK);
    assert(c.cpu.Wptr == 0x80000048u);
    assert(c.cpu.Oreg == 0u);
    assert(c.cycles == 1u);
}

static void test_eqc(void)
{
    t425_core_t c;

    t425_core_reset(&c);
    c.cpu.A = 5u;
    code(&c, 0xC5u);

    assert(t425_core_step(&c) == T425_STEP_OK);
    assert(c.cpu.A == 1u);
    assert(c.cpu.Oreg == 0u);

    t425_core_reset(&c);
    c.cpu.A = 7u;
    code(&c, 0xC5u);

    assert(t425_core_step(&c) == T425_STEP_OK);
    assert(c.cpu.A == 0u);
}

static void test_stl(void)
{
    t425_core_t c;
    uint32_t value;

    t425_core_reset(&c);
    c.cpu.Wptr = 0x80000040u;
    c.cpu.A = 0x11111111u;
    c.cpu.B = 0x22222222u;
    c.cpu.C = 0x33333333u;

    code(&c, 0xD2u);

    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(t425_memory_read_word(
        &c.memory, 0x80000048u, &value) == 0);
    assert(value == 0x11111111u);

    assert(c.cpu.A == 0x22222222u);
    assert(c.cpu.B == 0x33333333u);
    assert(c.cpu.Oreg == 0u);
}

static void test_stnl(void)
{
    t425_core_t c;
    uint32_t value;

    t425_core_reset(&c);
    c.cpu.A = 0x80000040u;
    c.cpu.B = 0x11111111u;
    c.cpu.C = 0x22222222u;

    code(&c, 0xE2u);

    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(t425_memory_read_word(
        &c.memory, 0x80000048u, &value) == 0);
    assert(value == 0x11111111u);

    assert(c.cpu.A == 0x22222222u);
    assert(c.cpu.B == 0x11111111u);
    assert(c.cpu.C == 0x22222222u);
    assert(c.cpu.Oreg == 0u);
}

int main(void)
{
    test_ajw();
    test_eqc();
    test_stl();
    test_stnl();

    puts("F08.1-A.3 stack/workspace: PASS");
    return 0;
}
