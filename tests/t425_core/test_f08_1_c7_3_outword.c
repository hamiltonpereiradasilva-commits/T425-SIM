#include <stdint.h>
#include "f1_fixture_compat.h"
#include <stdio.h>

#include "t425_core.h"

static int fail(const char *message)
{
    fprintf(stderr, "F08.1-C.7.3 OUTWORD: FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    t425_core_t core;
    uint32_t ip = T425_INTERNAL_RAM_BASE;
    uint32_t wptr = T425_INTERNAL_RAM_BASE + 0x120u;
    uint32_t old_a = 0x11223344u;
    uint32_t old_b = T425_INTERNAL_RAM_BASE + 0x180u;
    uint32_t stored = 0u;
    uint64_t cycles_before;
    uint64_t instructions_before;

    t425_core_reset(&core);

    core.cpu.Iptr = ip;
    core.cpu.Wptr = wptr;
    core.cpu.A = old_a; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = old_b; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0xCAFEBABEu; t425_fixture_sync_slaves(&core.cpu);

    /* PFIX 0 + OPR 0x0F = OUTWORD */
    if (t425_memory_write_byte(&core.memory, ip, 0x20u) != 0)
        return fail("cannot write PFIX");

    if (t425_memory_write_byte(&core.memory, ip + 1u, 0xFFu) != 0)
        return fail("cannot write OUTWORD");

    cycles_before = core.cycles;
    instructions_before = core.instructions;

    if (t425_core_step(&core) != T425_STEP_OK)
        return fail("PFIX execution failed");

    if (t425_core_step(&core) != T425_STEP_OK)
        return fail("OUTWORD execution failed");

    if (core.cpu.A != old_b)
        return fail("A != previous B");

    if (core.cpu.B != old_a)
        return fail("B != previous A");

    if (core.cpu.C != wptr)
        return fail("C != Wptr");

    if (t425_memory_read_word(&core.memory, wptr, &stored) != 0)
        return fail("cannot read OUTWORD destination");

    if (stored != old_b + 4u)
        return fail("MEM[Wptr] != B + 4");

    if (core.t425_data_reg != old_b + 4u)
        return fail("DataReg != B + 4");

    /* PFIX = 1 cycle + OUTWORD = 23 cycles */
    if (core.cycles - cycles_before != 24u)
        return fail("unexpected cycle count");

    if (core.instructions - instructions_before != 2u)
        return fail("unexpected instruction count");

    if (core.cpu.Iptr != ip + 2u)
        return fail("unexpected Iptr");

    if (core.cpu.Oreg != 0u)
        return fail("Oreg not cleared");

    printf("F08.1-C.7.3 OUTWORD: PASS\n");
    return 0;
}
