#include <stdio.h>
#include "f1_fixture_compat.h"
#include <stdint.h>

#include "t425_core.h"
#include "t425_channel.h"
#include "t425_memory.h"

static int execute_opr(t425_core_t *core, uint32_t opr)
{
    /*
     * t425_core_step() fetches from MEM[Iptr], then increments Iptr.
     *
     * The isolated CORE reset leaves Iptr at zero.  The architectural
     * internal RAM window begins at T425_INTERNAL_RAM_BASE, so the test
     * establishes a valid instruction pointer explicitly.
     */
    uint32_t ip = T425_INTERNAL_RAM_BASE;

    core->cpu.Iptr = ip;

    /*
     * T425 instruction stream:
     *
     *   0x21 = PFIX 1
     *   0xF2 = OPR 2
     *
     * Therefore Oreg becomes 0x12 = RESETCH.
     */
    if (t425_memory_write_byte(
            &core->memory,
            ip,
            (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu))) != 0)
        return T425_STEP_BADMEM;

    if (t425_memory_write_byte(
            &core->memory,
            ip + 1u,
            (uint8_t)(0xF0u | (opr & 0x0Fu))) != 0)
        return T425_STEP_BADMEM;

    if (t425_core_step(core) != T425_STEP_OK)
        return T425_STEP_BADOP;

    return t425_core_step(core);
}

int main(void)
{
    t425_core_t core;
    const uint32_t channel = 0x80000100u;
    const uint32_t waiting_wdesc = 0x00000201u;

    t425_core_reset(&core);

    /*
     * Channel control words are architectural memory state.
     * Establish the empty channel explicitly before installing a waiter.
     */
    if (t425_channel_reset(&core.memory, channel) != 0) {
        fprintf(stderr,
                "FAIL: could not reset channel before test\n");
        return 1;
    }

    if (t425_channel_wait(
            &core.memory,
            channel,
            waiting_wdesc) != 0) {
        fprintf(stderr,
                "FAIL: could not establish waiting channel\n");
        return 1;
    }

    /*
     * RESETCH consumes A as the architectural channel address.
     * B and C are sentinels to verify preservation.
     */
    core.cpu.A = channel; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x11223344u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x55667788u; t425_fixture_sync_slaves(&core.cpu);

    uint64_t cycles_before = core.cycles;

    int rc = execute_opr(&core, 0x12u);

    if (rc != T425_STEP_OK) {
        fprintf(stderr,
                "FAIL: RESETCH returned %d\n",
                rc);
        return 1;
    }

    uint32_t word = 0;

    if (t425_channel_read_word(
            &core.memory,
            channel,
            &word) != 0) {
        fprintf(stderr,
                "FAIL: could not read channel after RESETCH\n");
        return 1;
    }

    if (word != T425_CHANNEL_EMPTY) {
        fprintf(stderr,
                "FAIL: channel = 0x%08X, expected 0x%08X\n",
                word,
                T425_CHANNEL_EMPTY);
        return 1;
    }

    if (core.cpu.A != channel ||
        core.cpu.B != 0x11223344u ||
        core.cpu.C != 0x55667788u) {
        fprintf(stderr,
                "FAIL: RESETCH modified A/B/C unexpectedly: "
                "A=%08X B=%08X C=%08X\n",
                core.cpu.A,
                core.cpu.B,
                core.cpu.C);
        return 1;
    }

    /*
     * PFIX = 1 cycle
     * RESETCH = 3 cycles
     */
    if (core.cycles != cycles_before + 4u) {
        fprintf(stderr,
                "FAIL: total cycles=%llu, expected %llu "
                "(PFIX + RESETCH)\n",
                (unsigned long long)core.cycles,
                (unsigned long long)(cycles_before + 4u));
        return 1;
    }

    if (core.instructions != 2u) {
        fprintf(stderr,
                "FAIL: instructions=%llu, expected 2\n",
                (unsigned long long)core.instructions);
        return 1;
    }

    printf("F08.1-C.7.1 RESETCH: PASS\n");
    return 0;
}
