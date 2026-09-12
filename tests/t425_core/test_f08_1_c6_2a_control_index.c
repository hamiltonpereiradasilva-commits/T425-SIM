#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_memory.h"

#define CODE_BASE T425_INTERNAL_RAM_BASE

static void reset_code(t425_core_t *core)
{
    t425_core_reset(core);
    core->cpu.Iptr = CODE_BASE;
}

static void emit_op(t425_core_t *core, uint16_t code)
{
    uint8_t pfix;
    uint8_t opr;

    assert(code <= 0xFFu);

    pfix = (uint8_t)(0x20u | ((code >> 4u) & 0x0Fu));
    opr  = (uint8_t)(0xF0u | (code & 0x0Fu));

    assert(t425_memory_write_byte(&core->memory,
                                  CODE_BASE,
                                  pfix) == 0);
    assert(t425_memory_write_byte(&core->memory,
                                  CODE_BASE + 1u,
                                  opr) == 0);

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static uint32_t read_word(t425_core_t *core, uint32_t address)
{
    uint32_t value;

    assert(t425_memory_read_word(&core->memory,
                                 address,
                                 &value) == 0);

    return value;
}

static void write_word(t425_core_t *core,
                       uint32_t address,
                       uint32_t value)
{
    assert(t425_memory_write_word(&core->memory,
                                  address,
                                  value) == 0);
}

static void test_norm(void)
{
    t425_core_t core;

    reset_code(&core);

    core.cpu.A = 1u;
    core.cpu.B = 0u;

    emit_op(&core, 0x19u);

    assert(core.cpu.A == 0u);
    assert(core.cpu.B == 0x80000000u);
    assert(core.cpu.C == 63u);

    reset_code(&core);

    core.cpu.A = 0u;
    core.cpu.B = 0u;

    emit_op(&core, 0x19u);

    assert(core.cpu.A == 0u);
    assert(core.cpu.B == 0u);
    assert(core.cpu.C == 64u);
}

static void test_lend(void)
{
    t425_core_t core;
    uint32_t block = CODE_BASE + 0x100u;

    reset_code(&core);

    write_word(&core, block, 10u);
    write_word(&core, block + 4u, 2u);

    core.cpu.A = 2u;
    core.cpu.B = block;

    emit_op(&core, 0x21u);

    assert(read_word(&core, block) == 11u);
    assert(read_word(&core, block + 4u) == 1u);

    /*
     * The current instruction model does not execute a synthetic
     * branch target here; exercise the exit case independently.
     */
    reset_code(&core);

    write_word(&core, block, 10u);
    write_word(&core, block + 4u, 1u);

    core.cpu.A = 2u;
    core.cpu.B = block;

    emit_op(&core, 0x21u);

    assert(read_word(&core, block) == 10u);
    assert(read_word(&core, block + 4u) == 0u);
}

static void test_gajw(void)
{
    t425_core_t core;

    reset_code(&core);

    core.cpu.A = CODE_BASE + 0x123u;
    core.cpu.Wptr = CODE_BASE + 0x300u;
    core.cpu.B = 0x11111111u;
    core.cpu.C = 0x22222222u;

    emit_op(&core, 0x3Cu);

    assert(core.cpu.A == CODE_BASE + 0x300u);
    assert(core.cpu.Wptr ==
           ((CODE_BASE + 0x123u) & ~3u));
    assert(core.cpu.B == 0x11111111u);
    assert(core.cpu.C == 0x22222222u);
}

static void test_move(void)
{
    t425_core_t core;
    uint32_t src = CODE_BASE + 0x100u;
    uint32_t dst = CODE_BASE + 0x180u;
    uint32_t i;
    uint8_t value;

    reset_code(&core);

    for (i = 0u; i < 13u; ++i)
    {
        assert(t425_memory_write_byte(&core.memory,
                                      src + i,
                                      (uint8_t)(0xA0u + i)) == 0);
    }

    core.cpu.A = 13u;
    core.cpu.B = dst;
    core.cpu.C = src;

    emit_op(&core, 0x4Au);

    for (i = 0u; i < 13u; ++i)
    {
        assert(t425_memory_read_byte(&core.memory,
                                     dst + i,
                                     &value) == 0);

        assert(value == (uint8_t)(0xA0u + i));
    }
}

static void test_ccnt1(void)
{
    t425_core_t core;

    reset_code(&core);

    core.cpu.A = 10u;
    core.cpu.B = 7u;
    core.cpu.C = 0x12345678u;

    emit_op(&core, 0x4Du);

    assert(core.cpu.A == 7u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.error == 0u);

    reset_code(&core);

    core.cpu.A = 10u;
    core.cpu.B = 0u;
    core.cpu.C = 0xCAFEBABEu;

    emit_op(&core, 0x4Du);

    assert(core.cpu.A == 0u);
    assert(core.cpu.B == 0xCAFEBABEu);
    assert(core.cpu.error != 0u);

    reset_code(&core);

    core.cpu.A = 10u;
    core.cpu.B = 11u;
    core.cpu.C = 0xCAFEBABEu;

    emit_op(&core, 0x4Du);

    assert(core.cpu.A == 11u);
    assert(core.cpu.B == 0xCAFEBABEu);
    assert(core.cpu.error != 0u);
}

int main(void)
{
    test_norm();
    test_lend();
    test_gajw();
    test_move();
    test_ccnt1();

    puts("F08.1-C.6.2-A control/index operations: PASS");
    return 0;
}
