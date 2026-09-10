#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void code(t425_core_t *c, uint8_t b)
{
    assert(t425_memory_write_byte(
        &c->memory, 0x80000000u, b) == 0);
    c->cpu.Iptr = 0x80000000u;
}

static void code2(t425_core_t *c, uint8_t b0, uint8_t b1)
{
    assert(t425_memory_write_byte(
        &c->memory, 0x80000000u, b0) == 0);
    assert(t425_memory_write_byte(
        &c->memory, 0x80000001u, b1) == 0);
    c->cpu.Iptr = 0x80000000u;
}

static void test_call(void)
{
    t425_core_t c;
    uint32_t value;

    t425_core_reset(&c);

    c.cpu.Wptr = 0x80000080u;
    c.cpu.Iptr = 0x80000000u;

    c.cpu.A = 0x11111111u;
    c.cpu.B = 0x22222222u;
    c.cpu.C = 0x33333333u;

    code(&c, 0x91u); /* CALL #1 */

    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(c.cpu.Wptr == 0x80000070u);
    assert(c.cpu.A == 0x80000001u);
    assert(c.cpu.Iptr == 0x80000002u);

    assert(t425_memory_read_word(
        &c.memory, 0x80000070u, &value) == 0);
    assert(value == 0x80000001u);

    assert(t425_memory_read_word(
        &c.memory, 0x80000074u, &value) == 0);
    assert(value == 0x11111111u);

    assert(t425_memory_read_word(
        &c.memory, 0x80000078u, &value) == 0);
    assert(value == 0x22222222u);

    assert(t425_memory_read_word(
        &c.memory, 0x8000007Cu, &value) == 0);
    assert(value == 0x33333333u);

    assert(c.cpu.Oreg == 0u);
    assert(c.cycles == 7u);
}

static void test_ret(void)
{
    t425_core_t c;

    t425_core_reset(&c);

    c.cpu.Wptr = 0x80000070u;
    c.cpu.A = 0xAAAAAAAAu;
    c.cpu.B = 0xBBBBBBBBu;
    c.cpu.C = 0xCCCCCCCCu;

    assert(t425_memory_write_word(
        &c.memory, 0x80000070u, 0x80000123u) == 0);

    code2(&c, 0x22u, 0xF0u); /* PFIX #2, OPR #0 = RET 0x20 */

    assert(t425_core_step(&c) == T425_STEP_OK);
    assert(c.cpu.Oreg == 0x20u);
    assert(c.cpu.Iptr == 0x80000001u);
    assert(c.cycles == 1u);

    assert(t425_core_step(&c) == T425_STEP_OK);
    assert(c.cpu.Oreg == 0u);
    assert(c.cpu.Iptr == 0x80000123u);
    assert(c.cpu.Wptr == 0x80000080u);
    assert(c.cpu.A == 0xAAAAAAAAu);
    assert(c.cpu.B == 0xBBBBBBBBu);
    assert(c.cpu.C == 0xCCCCCCCCu);
    assert(c.cycles == 6u);
    assert(c.instructions == 2u);
}

int main(void)
{
    test_call();
    test_ret();

    puts("F08.1-A.5 CALL/RET preliminary: PASS");
    return 0;
}
