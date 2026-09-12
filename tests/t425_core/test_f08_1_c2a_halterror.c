#include <assert.h>
#include <stdio.h>

#include "t425_core.h"

static int opr(t425_core_t *core, unsigned code)
{
    uint32_t a = T425_INTERNAL_RAM_BASE;

    core->cpu.Iptr = a;

    assert(t425_memory_write_byte(
        &core->memory, a,
        (uint8_t)(0x20u | ((code >> 4) & 0x0Fu))) == 0);

    assert(t425_memory_write_byte(
        &core->memory, a + 1u,
        (uint8_t)(0xF0u | (code & 0x0Fu))) == 0);

    assert(t425_core_step(core) == T425_STEP_OK);
    return t425_core_step(core);
}

static void test_set_test_clear(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    assert(core.cpu.halt_on_error == 0u);

    assert(opr(&core, 0x58u) == T425_STEP_OK);
    assert(core.cpu.halt_on_error == 1u);

    assert(opr(&core, 0x59u) == T425_STEP_OK);
    assert(core.cpu.A == 1u);
    assert(core.cpu.halt_on_error == 1u);

    assert(opr(&core, 0x57u) == T425_STEP_OK);
    assert(core.cpu.halt_on_error == 0u);

    assert(opr(&core, 0x59u) == T425_STEP_OK);
    assert(core.cpu.A == 0u);
    assert(core.cpu.halt_on_error == 0u);
}

int main(void)
{
    test_set_test_clear();

    puts("F08.1-C.2a HALTERR flags: PASS");
    return 0;
}
