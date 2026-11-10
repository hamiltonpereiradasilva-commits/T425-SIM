
#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_channel.h"
#include "t425_process.h"

static void setup_process(
    t425_core_t *core,
    uint32_t wptr,
    uint32_t iptr)
{
    assert(t425_memory_write_word(
        &core->memory, wptr - 4u, iptr) == 0);
    assert(t425_memory_write_word(
        &core->memory, wptr - 8u, T425_NOT_PROCESS) == 0);
    assert(t425_memory_write_word(
        &core->memory, wptr - 12u,
        T425_PROCESS_STATE_ENABLING) == 0);
    assert(t425_memory_write_word(
        &core->memory, wptr - 16u,
        T425_TIMER_TIME_SET) == 0);
    assert(t425_memory_write_word(
        &core->memory, wptr - 20u, 0u) == 0);
}

static uint8_t read_byte(
    t425_core_t *core,
    uint32_t address)
{
    uint8_t value;
    assert(t425_memory_read_byte(
        &core->memory, address, &value) == 0);
    return value;
}

static void test_out_then_in(void)
{
    t425_core_t core;

    const uint32_t p1 =
        T425_INTERNAL_RAM_BASE + 0x100u;
    const uint32_t p2 =
        T425_INTERNAL_RAM_BASE + 0x200u;

    const uint32_t p1_iptr =
        T425_INTERNAL_RAM_BASE + 0x300u;
    const uint32_t p2_iptr =
        T425_INTERNAL_RAM_BASE + 0x400u;

    const uint32_t channel =
        T425_INTERNAL_RAM_BASE + 0x500u;

    const uint32_t source =
        T425_INTERNAL_RAM_BASE + 0x600u;
    const uint32_t destination =
        T425_INTERNAL_RAM_BASE + 0x700u;

    const uint32_t p1_wdesc =
        p1 | T425_PRIORITY_HIGH;
    const uint32_t p2_wdesc =
        p2 | T425_PRIORITY_HIGH;

    uint32_t channel_word;

    t425_core_reset(&core);

    setup_process(&core, p1, p1_iptr);
    setup_process(&core, p2, p2_iptr);

    assert(t425_channel_reset(
        &core.memory, channel) == 0);

    /*
     * OUT = PFIX 0 + OPR 0x0B.
     */
    assert(t425_memory_write_byte(
        &core.memory, p1_iptr, 0x20u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, p1_iptr + 1u, 0xFBu) == 0);

    /*
     * IN = PFIX 0 + OPR 0x07.
     */
    assert(t425_memory_write_byte(
        &core.memory, p2_iptr, 0x20u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, p2_iptr + 1u, 0xF7u) == 0);

    /*
     * P1 starts executing OUT.
     */
    core.scheduler.current_wdesc = p1_wdesc;
    core.scheduler.running = 1u;
    core.cpu.Wptr = p1;
    core.cpu.Iptr = p1_iptr;
    core.cpu.A = 4u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = channel; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = source; t425_fixture_sync_slaves(&core.cpu);

    assert(t425_memory_write_byte(
        &core.memory, source + 0u, 0x11u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, source + 1u, 0x22u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, source + 2u, 0x33u) == 0);
    assert(t425_memory_write_byte(
        &core.memory, source + 3u, 0x44u) == 0);

    /*
     * P2 is already runnable.
     */
    core.scheduler.high_head = p2_wdesc;
    core.scheduler.high_tail = p2_wdesc;

    /*
     * Execute PFIX + OUT.
     */
    {
        int rc;

        rc = t425_core_step(&core);
        printf("DEBUG PFIX: rc=%d Wptr=0x%08X Iptr=0x%08X A=0x%08X B=0x%08X C=0x%08X cycles=%llu\\n",
               rc, core.cpu.Wptr, core.cpu.Iptr,
               core.cpu.A, core.cpu.B, core.cpu.C,
               (unsigned long long)core.cycles);
        assert(rc == T425_STEP_OK);

        rc = t425_core_step(&core);
        printf("DEBUG OUT: rc=%d Wptr=0x%08X Iptr=0x%08X A=0x%08X B=0x%08X C=0x%08X cycles=%llu\\n",
               rc, core.cpu.Wptr, core.cpu.Iptr,
               core.cpu.A, core.cpu.B, core.cpu.C,
               (unsigned long long)core.cycles);
        assert(rc == T425_STEP_OK);
    }

    /*
     * OUT must block P1 and schedule P2.
     */
    assert(core.cpu.Wptr == p2);
    assert(core.cpu.Iptr == p2_iptr);

    assert(t425_channel_read_word(
        &core.memory, channel, &channel_word) == 0);
    assert(channel_word == p1_wdesc);

    /*
     * P2 executes IN.
     */
    core.cpu.A = 4u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = channel; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = destination; t425_fixture_sync_slaves(&core.cpu);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    /*
     * Rendezvous consumed the channel waiter.
     */
    assert(t425_channel_read_word(
        &core.memory, channel, &channel_word) == 0);
    assert(channel_word == T425_CHANNEL_EMPTY);

    /*
     * Verify direct architectural-memory transfer.
     */
    assert(read_byte(&core, destination + 0u) == 0x11u);
    assert(read_byte(&core, destination + 1u) == 0x22u);
    assert(read_byte(&core, destination + 2u) == 0x33u);
    assert(read_byte(&core, destination + 3u) == 0x44u);

    /*
     * Both participants were made runnable again.
     */
    assert(core.scheduler.current_wdesc != T425_NOT_PROCESS);

    puts("F08.1-B IN/OUT rendezvous transfer: PASS");
}

int main(void)
{
    test_out_then_in();
    return 0;
}
