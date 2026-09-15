#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_channel.h"
#include "t425_process.h"

#define RAM_BASE 0x80000000u

static void setup_process(t425_core_t *core,
                          uint32_t wptr,
                          uint32_t iptr,
                          uint32_t wdesc)
{
    assert(t425_memory_write_word(&core->memory,
                                  wptr - 4u, iptr) == 0);
    assert(t425_memory_write_word(&core->memory,
                                  wptr - 8u,
                                  T425_NOT_PROCESS) == 0);
    assert(t425_memory_write_word(&core->memory,
                                  wptr - 12u,
                                  T425_PROCESS_STATE_READY) == 0);
    assert(t425_memory_write_word(&core->memory,
                                  wptr - 16u, 0u) == 0);
    assert(t425_memory_write_word(&core->memory,
                                  wptr - 20u, 0u) == 0);
    assert(t425_memory_write_word(&core->memory,
                                  wptr - 24u, 0u) == 0);
    (void)wdesc;
}

static void load_code(t425_core_t *core,
                      uint32_t iptr,
                      uint8_t opr)
{
    assert(t425_memory_write_byte(&core->memory,
                                  iptr, 0x20u) == 0);
    assert(t425_memory_write_byte(&core->memory,
                                  iptr + 1u, opr) == 0);
}

int main(void)
{
    t425_core_t core;

    const uint32_t p_in_wptr  = RAM_BASE + 0x100u;
    const uint32_t p_out_wptr = RAM_BASE + 0x200u;
    const uint32_t p_in_iptr  = RAM_BASE + 0x300u;
    const uint32_t p_out_iptr = RAM_BASE + 0x400u;
    const uint32_t channel    = RAM_BASE + 0x500u;
    const uint32_t source     = RAM_BASE + 0x600u;
    const uint32_t destination = RAM_BASE + 0x700u;

    const uint32_t in_wdesc =
        p_in_wptr | T425_PRIORITY_HIGH;
    const uint32_t out_wdesc =
        p_out_wptr | T425_PRIORITY_HIGH;

    uint32_t value;
    uint32_t i;

    t425_core_reset(&core);

    setup_process(&core, p_in_wptr, p_in_iptr, in_wdesc);
    setup_process(&core, p_out_wptr, p_out_iptr, out_wdesc);

    load_code(&core, p_in_iptr, 0xF7u);
    load_code(&core, p_out_iptr, 0xFBu);

    assert(t425_channel_reset(&core.memory, channel) == 0);

    /* Put OUT process on the high-priority ready queue. */
    core.scheduler.high_head = out_wdesc;
    core.scheduler.high_tail = out_wdesc;

    /*
     * Case 1: IN arrives first and must block.
     */
    core.scheduler.current_wdesc = in_wdesc;
    core.scheduler.running = 1u;
    core.cpu.Wptr = p_in_wptr;
    core.cpu.Iptr = p_in_iptr;
    core.cpu.A = 4u;
    core.cpu.B = channel;
    core.cpu.C = destination;

    assert(t425_memory_write_word(&core.memory,
                                  source, 0x44332211u) == 0);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.scheduler.current_wdesc == out_wdesc);
    assert(core.cpu.Wptr == p_out_wptr);

    assert(t425_channel_read_word(&core.memory,
                                  channel, &value) == 0);
    assert(value == in_wdesc);

    /*
     * OUT completes the waiting IN.
     */
    core.cpu.A = 4u;
    core.cpu.B = channel;
    core.cpu.C = source;

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    for (i = 0u; i < 4u; ++i)
    {
        uint8_t b;

        assert(t425_memory_read_byte(&core.memory,
                                     destination + i, &b) == 0);
        assert(b == (uint8_t)(0x11u + (i * 0x11u)));
    }

    assert(t425_channel_read_word(&core.memory,
                                  channel, &value) == 0);
    assert(value == T425_CHANNEL_EMPTY);

    /*
     * The blocked IN process must have returned to the scheduler.
     */
    assert(core.scheduler.high_head != T425_NOT_PROCESS ||
           core.scheduler.low_head != T425_NOT_PROCESS);

    /*
     * Reuse the same channel for a second rendezvous.
     */
    t425_core_reset(&core);

    setup_process(&core, p_in_wptr, p_in_iptr, in_wdesc);
    setup_process(&core, p_out_wptr, p_out_iptr, out_wdesc);

    load_code(&core, p_in_iptr, 0xF7u);
    load_code(&core, p_out_iptr, 0xFBu);

    assert(t425_channel_reset(&core.memory, channel) == 0);

    /* Put IN process on the high-priority ready queue. */
    core.scheduler.high_head = in_wdesc;
    core.scheduler.high_tail = in_wdesc;

    core.scheduler.current_wdesc = out_wdesc;
    core.scheduler.running = 1u;
    core.cpu.Wptr = p_out_wptr;
    core.cpu.Iptr = p_out_iptr;
    core.cpu.A = 0u;
    core.cpu.B = channel;
    core.cpu.C = source;

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.scheduler.current_wdesc == in_wdesc);

    core.cpu.A = 0u;
    core.cpu.B = channel;
    core.cpu.C = destination;

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(t425_channel_read_word(&core.memory,
                                  channel, &value) == 0);
    assert(value == T425_CHANNEL_EMPTY);

    printf("F08.1-B IN/OUT edge cases: PASS\n");
    return 0;
}
