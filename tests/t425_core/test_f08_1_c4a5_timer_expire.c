#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "t425_core.h"
#include "t425_memory.h"
#include "t425_process.h"

#define W0 0x80000100u
#define W1 0x80000140u

static void fail(const char *msg)
{
    fprintf(stderr, "FAIL: %s\n", msg);
    exit(1);
}

static void expect_u32(const char *name, uint32_t got, uint32_t expected)
{
    if (got != expected)
    {
        fprintf(stderr, "FAIL: %s got=0x%08X expected=0x%08X\n",
                name, got, expected);
        exit(1);
    }
}

static void expect_i(const char *name, int got, int expected)
{
    if (got != expected)
    {
        fprintf(stderr, "FAIL: %s got=%d expected=%d\n",
                name, got, expected);
        exit(1);
    }
}

static void put_program(t425_core_t *core, uint32_t iptr,
                        uint8_t b0, uint8_t b1)
{
    if (t425_memory_write_byte(&core->memory, iptr, b0) != 0 ||
        t425_memory_write_byte(&core->memory, iptr + 1u, b1) != 0)
        fail("program write");
}

static void test_low_expiration(void)
{
    t425_core_t core;
    uint32_t value;
    int rc;

    t425_core_reset(&core);

    core.cpu.Wptr = W1;
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.scheduler.current_wdesc = W1 | T425_PRIORITY_LOW;
    core.scheduler.running = 1u;

    core.timer.low = 119u;
    core.timer.high = 100u;

    /* C4-A.5 exercises an already-running timer clock. */
    core.timer.running = 1u;

    core.timer.low_cycle_accum = 1279u;

    if (t425_memory_write_word(&core.memory, W0 - 20u, 120u) != 0)
        fail("W0 Time write");

    if (t425_memory_write_word(&core.memory, W0 - 16u,
                               T425_NOT_PROCESS) != 0)
        fail("W0 TLink write");

    if (t425_memory_write_word(&core.memory, W0 - 12u,
                               T425_PROCESS_STATE_WAITING) != 0)
        fail("W0 State write");

    if (t425_memory_write_word(&core.memory, T425_TPTR_LOC1,
                               W0 | T425_PRIORITY_LOW) != 0)
        fail("TPtrLoc1 write");

    core.timer.next_low = 120u;

    if (t425_memory_write_word(&core.memory, W1 - 8u,
                               T425_NOT_PROCESS) != 0)
        fail("W1 Link write");

    if (t425_memory_write_word(&core.memory, W1 - 12u,
                               T425_PROCESS_STATE_READY) != 0)
        fail("W1 State write");

    put_program(&core, T425_INTERNAL_RAM_BASE, 0x22u, 0xF2u);

    rc = t425_core_step(&core);
    expect_i("expiration LDTIMER PFIX", rc, T425_STEP_OK);

    rc = t425_core_step(&core);
    expect_i("expiration LDTIMER", rc, T425_STEP_OK);

    expect_u32("expiration low clock", core.timer.low, 120u);
    if (t425_memory_read_word(&core.memory, W0 - 12u, &value) != 0)
        fail("expiration W0 state read");

    expect_u32("expiration W0 state", value,
               T425_PROCESS_STATE_READY);

    if (t425_memory_read_word(&core.memory, T425_TPTR_LOC1, &value) != 0)
        fail("expiration TPtrLoc1 read");

    expect_u32("expiration TPtrLoc1", value, T425_NOT_PROCESS);

    expect_u32("expiration TNext low", core.timer.next_low,
               UINT32_MAX);
    expect_u32("expiration ready queue head",
               core.scheduler.low_head,
               W0 | T425_PRIORITY_LOW);
    expect_u32("expiration ready queue tail",
               core.scheduler.low_tail,
               W0 | T425_PRIORITY_LOW);
if (t425_memory_read_word(&core.memory, W0 - 8u, &value) != 0)
        fail("expiration W0 ready link read");

    expect_u32("expiration W0 tail link", value,
               T425_NOT_PROCESS);

    printf("C4-A.5 LOW expiration/wakeup: PASS\n");
}

int main(void)
{
    test_low_expiration();
    printf("F08.1-C.4a.5 timer expiration tests: PASS\n");
    return 0;
}
