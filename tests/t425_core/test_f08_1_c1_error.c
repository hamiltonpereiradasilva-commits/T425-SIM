#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void reset_core(t425_core_t *core)
{
    t425_core_reset(core);
    core->cpu.Iptr = T425_INTERNAL_RAM_BASE;
}

static int step_opr(t425_core_t *core, uint32_t opr)
{
    uint32_t address = T425_INTERNAL_RAM_BASE;

    core->cpu.Iptr = address;

    assert(t425_memory_write_byte(
        &core->memory,
        address,
        (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu))) == 0);

    assert(t425_memory_write_byte(
        &core->memory,
        address + 1u,
        (uint8_t)(0xF0u | (opr & 0x0Fu))) == 0);

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(core->cpu.Oreg == (opr & 0xF0u));

    return t425_core_step(core);
}

int main(void)
{
    t425_core_t core;

    /*
     * TESTERR with no error:
     * pushes false and takes the documented no-error timing path.
     */
    reset_core(&core);
    assert(core.cpu.error == 0u);

    assert(step_opr(&core, 0x29u) == T425_STEP_OK);
    assert(core.cpu.A == 0u);
    assert(core.cpu.error == 0u);
    assert(core.cycles == 3u);

    /*
     * SETERR establishes the internal error flag.
     */
    reset_core(&core);

    assert(step_opr(&core, 0x10u) == T425_STEP_OK);
    assert(core.cpu.error == 1u);
    assert(core.cycles == 2u);

    /*
     * TESTERR reports the error and clears it.
     */
    assert(step_opr(&core, 0x29u) == T425_STEP_OK);
    assert(core.cpu.A == 1u);
    assert(core.cpu.error == 0u);

    /*
     * A second TESTERR now reports no error.
     */
    assert(step_opr(&core, 0x29u) == T425_STEP_OK);
    assert(core.cpu.A == 0u);
    assert(core.cpu.error == 0u);

    puts("F08.1-C.1 SETERR/TESTERR: PASS");
    return 0;
}
