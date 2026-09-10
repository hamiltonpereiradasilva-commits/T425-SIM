#include <assert.h>
#include <stdio.h>
#include "t425_core.h"

static void code(t425_core_t *c, uint8_t b)
{
    assert(t425_memory_write_byte(
        &c->memory, 0x80000000u, b) == 0);
    c->cpu.Iptr = 0x80000000u;
}

static void test_j(void)
{
    t425_core_t c;

    t425_core_reset(&c);

    c.cpu.A = 0x11111111u;
    c.cpu.B = 0x22222222u;
    c.cpu.C = 0x33333333u;

    code(&c, 0x02u);

    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(c.cpu.Iptr == 0x80000003u);
    assert(c.cycles == 3u);
}

static void test_cj_taken(void)
{
    t425_core_t c;

    t425_core_reset(&c);

    c.cpu.A = 0u;
    c.cpu.B = 0x11111111u;
    c.cpu.C = 0x22222222u;

    code(&c, 0xA2u);

    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(c.cpu.Iptr == 0x80000003u);
    assert(c.cpu.A == 0u);
    assert(c.cpu.B == 0x11111111u);
    assert(c.cpu.C == 0x22222222u);
    assert(c.cycles == 4u);
}

static void test_cj_not_taken(void)
{
    t425_core_t c;

    t425_core_reset(&c);

    c.cpu.A = 1u;
    c.cpu.B = 0x11111111u;
    c.cpu.C = 0x22222222u;

    code(&c, 0xA2u);

    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(c.cpu.Iptr == 0x80000001u);
    assert(c.cpu.A == 0x11111111u);
    assert(c.cpu.B == 0x22222222u);
    assert(c.cycles == 2u);
}

int main(void)
{
    test_j();
    test_cj_taken();
    test_cj_not_taken();

    puts("F08.1-A.4 control-flow: PASS");
    return 0;
}
