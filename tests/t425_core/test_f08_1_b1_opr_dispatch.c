#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void code(t425_core_t *core, uint32_t address, uint8_t byte)
{
    assert(t425_memory_write_byte(
        &core->memory,
        address,
        byte) == 0);
}

/*
 * B.1: OPR dispatch must preserve RET (0x20).
 *
 * CALL stores the return Iptr at the new workspace pointer.
 * RET must recover that Iptr through the OPR dispatcher.
 */
static void test_ret_dispatch(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.Wptr = 0x80000100u;
    core.cpu.Iptr = 0x80000000u;

    /*
     * The instruction stream contains OPR 0x20:
     * direct OPR + operand construction.
     *
     * OPR 0x20 is encoded as F2 00:
     * F2 = PFIX with operand 2
     * 00 = OPR with operand 0
     *
     * Resulting Oreg = 0x20.
     */
    code(&core, 0x80000000u, 0x22u);
    code(&core, 0x80000001u, 0xF0u);

    /*
     * RET reads W[0] as the return instruction pointer.
     */
    assert(t425_memory_write_word(
        &core.memory,
        core.cpu.Wptr,
        0x80000080u) == 0);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(core.cpu.Oreg == 0x20u);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(core.cpu.Iptr == 0x80000080u);
    assert(core.cpu.Wptr == 0x80000110u);
    assert(core.cycles == 6u);
    assert(core.instructions == 2u);
}

/*
 * B.1: OPR dispatcher must reject an operation that has not
 * yet been implemented.
 */
static void test_unimplemented_opr_dispatch(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = 0x80000000u;

    /*
     * OPR 0x11 is not yet implemented.
     *
     * 0x21 0xF1 = PFIX 1 followed by OPR 1,
     * producing Oreg = 0x11.
     */
    code(&core, 0x80000000u, 0x21u);
    code(&core, 0x80000001u, 0xF1u);

    /*
     * PFIX 1 is executed first, producing Oreg = 0x10.
     */
    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(core.cpu.Oreg == 0x10u);
    assert(core.cycles == 1u);
    assert(core.instructions == 1u);

    /*
     * OPR 1 completes Oreg = 0x11. No implementation exists
     * for OPR 0x11, so the dispatcher must return UNIMPL.
     */
    assert(t425_core_step(&core) == T425_STEP_UNIMPL);

    /*
     * The dispatcher must not account the unimplemented
     * instruction as completed.
     */
    assert(core.cycles == 1u);
    assert(core.instructions == 1u);
}

int main(void)
{
    test_ret_dispatch();
    test_unimplemented_opr_dispatch();

    puts("F08.1-B.1 OPR dispatch tests: PASS");
    return 0;
}
