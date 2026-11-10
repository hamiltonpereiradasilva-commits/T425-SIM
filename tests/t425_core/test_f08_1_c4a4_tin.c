#include <stdio.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdlib.h>

#include "t425_core.h"
#include "t425_memory.h"
#include "t425_process.h"

#define W0 0x80000100u
#define W1 0x80000140u
#define W2 0x80000180u

static void fail(const char *msg)
{
    fprintf(stderr, "FAIL: %s\n", msg);
    exit(1);
}

static void expect_u32(const char *name, uint32_t got, uint32_t expected)
{
    if (got != expected)
    {
        fprintf(stderr,
                "FAIL: %s got=0x%08X expected=0x%08X\n",
                name, got, expected);
        exit(1);
    }
}

static void expect_i(const char *name, int got, int expected)
{
    if (got != expected)
    {
        fprintf(stderr,
                "FAIL: %s got=%d expected=%d\n",
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

/*
 * OPR 0x2B = TIN.
 * The test program is PFIX 2 / OPR 0xB => 0x22 0xFB.
 */
static void setup_core(t425_core_t *core, uint32_t wptr,
                       uint32_t wdesc, uint32_t now)
{
    t425_core_reset(core);

    core->cpu.Wptr = wptr;
    core->cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core->scheduler.current_wdesc = wdesc;
    core->scheduler.running = 1u;

    core->timer.high = now;
    core->timer.low = now;

    put_program(core, core->cpu.Iptr, 0x22u, 0xFBu);
}

static void test_past(void)
{
    t425_core_t core;
    uint32_t value;
    int rc;

    setup_core(&core, W0, W0 | T425_PRIORITY_LOW, 100u);
    core.cpu.A = 90u; t425_fixture_sync_slaves(&core.cpu);

    rc = t425_core_step(&core);
    expect_i("TIN past PFIX return", rc, T425_STEP_OK);
    expect_u32("TIN past PFIX cycles", (uint32_t)core.cycles, 1u);

    rc = t425_core_step(&core);
    expect_i("TIN past return", rc, T425_STEP_OK);
    expect_u32("TIN past total cycles", (uint32_t)core.cycles, 5u);

    if (t425_memory_read_word(&core.memory, T425_TPTR_LOC0, &value) != 0)
        fail("TIN past TPtrLoc0 read");
    expect_u32("TIN past TPtrLoc0", value, T425_NOT_PROCESS);

    printf("TIN past: PASS\n");
}

static void test_future_low_single(void)
{
    t425_core_t core;
    uint32_t value;
    int rc;

    setup_core(&core, W0, W0 | T425_PRIORITY_LOW, 100u);
    core.cpu.A = 120u; t425_fixture_sync_slaves(&core.cpu);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    if (t425_memory_write_word(&core.memory, W1 - 4u, 0x80000080u) != 0)
        fail("successor Iptr write");
    if (t425_memory_write_word(&core.memory, W1 - 8u, T425_NOT_PROCESS) != 0)
        fail("successor Link write");

    core.scheduler.low_head = W1 | T425_PRIORITY_LOW;
    core.scheduler.low_tail = W1 | T425_PRIORITY_LOW;

    rc = t425_core_step(&core);
    expect_i("TIN future PFIX return", rc, T425_STEP_OK);
    expect_u32("TIN future PFIX cycles", (uint32_t)core.cycles, 1u);

    rc = t425_core_step(&core);
    expect_i("TIN future return", rc, T425_STEP_OK);
    expect_u32("TIN future total cycles", (uint32_t)core.cycles, 31u);

    if (t425_memory_read_word(&core.memory, T425_TPTR_LOC1, &value) != 0)
        fail("TIN future TPtrLoc1 read");
    expect_u32("TIN future TPtrLoc1", value, W0 | T425_PRIORITY_LOW);

    if (t425_memory_read_word(&core.memory, W0 - 20u, &value) != 0)
        fail("TIN future Time read");
    expect_u32("TIN future Time", value, 120u);

    if (t425_memory_read_word(&core.memory, W0 - 16u, &value) != 0)
        fail("TIN future TLink read");
    expect_u32("TIN future TLink", value, T425_NOT_PROCESS);

    if (t425_memory_read_word(&core.memory, W0 - 12u, &value) != 0)
        fail("TIN future State read");
    expect_u32("TIN future State", value, T425_PROCESS_STATE_WAITING);

    if (t425_memory_read_word(&core.memory, W0 - 4u, &value) != 0)
        fail("TIN future Iptr read");
    expect_u32("TIN future saved Iptr", value, T425_INTERNAL_RAM_BASE + 2u);

    expect_u32("TIN future CPU Wptr", core.cpu.Wptr, W1);
    expect_u32("TIN future current wdesc", core.scheduler.current_wdesc,
               W1 | T425_PRIORITY_LOW);
    expect_u32("TIN future running", core.scheduler.running, 1u);

    printf("TIN future low single: PASS\n");
}

static void test_future_low_ordered(void)
{
    t425_core_t core;
    uint32_t value;
    int rc;

    setup_core(&core, W0, W0 | T425_PRIORITY_LOW, 100u);

    if (t425_memory_write_word(&core.memory, W1 - 4u, 0x80000080u) != 0)
        fail("ordered W1 Iptr write");
    if (t425_memory_write_word(&core.memory, W1 - 8u,
                               W2 | T425_PRIORITY_LOW) != 0)
        fail("ordered W1 Link write");
    if (t425_memory_write_word(&core.memory, W2 - 4u, 0x800000C0u) != 0)
        fail("ordered W2 Iptr write");
    if (t425_memory_write_word(&core.memory, W2 - 8u,
                               T425_NOT_PROCESS) != 0)
        fail("ordered W2 Link write");

    core.scheduler.low_head = W1 | T425_PRIORITY_LOW;
    core.scheduler.low_tail = W2 | T425_PRIORITY_LOW;

    core.cpu.A = 150u; t425_fixture_sync_slaves(&core.cpu);
    rc = t425_core_step(&core);
    expect_i("ordered W0 PFIX return", rc, T425_STEP_OK);
    rc = t425_core_step(&core);
    expect_i("ordered W0 TIN return", rc, T425_STEP_OK);
    expect_u32("ordered after W0 cycles", (uint32_t)core.cycles, 31u);
    expect_u32("ordered after W0 current", core.scheduler.current_wdesc,
               W1 | T425_PRIORITY_LOW);

    core.cpu.A = 200u; t425_fixture_sync_slaves(&core.cpu);
    put_program(&core, 0x80000080u, 0x22u, 0xFBu);
    rc = t425_core_step(&core);
    expect_i("ordered W1 PFIX return", rc, T425_STEP_OK);
    rc = t425_core_step(&core);
    expect_i("ordered W1 TIN return", rc, T425_STEP_OK);
    expect_u32("ordered after W1 cycles", (uint32_t)core.cycles, 62u);
    expect_u32("ordered after W1 current", core.scheduler.current_wdesc,
               W2 | T425_PRIORITY_LOW);

    core.cpu.A = 175u; t425_fixture_sync_slaves(&core.cpu);
    put_program(&core, 0x800000C0u, 0x22u, 0xFBu);
    rc = t425_core_step(&core);
    expect_i("ordered W2 PFIX return", rc, T425_STEP_OK);
    rc = t425_core_step(&core);
    expect_i("ordered W2 TIN return", rc, T425_STEP_OK);
    expect_u32("ordered after W2 cycles", (uint32_t)core.cycles, 93u);

    if (t425_memory_read_word(&core.memory, T425_TPTR_LOC1, &value) != 0)
        fail("ordered TPtrLoc1 read");
    expect_u32("ordered TPtrLoc1 head", value,
               W0 | T425_PRIORITY_LOW);

    if (t425_memory_read_word(&core.memory, W0 - 16u, &value) != 0)
        fail("ordered W0 TLink read");
    expect_u32("ordered W0 TLink", value, W2 | T425_PRIORITY_LOW);

    if (t425_memory_read_word(&core.memory, W2 - 20u, &value) != 0)
        fail("ordered W2 Time read");
    expect_u32("ordered W2 Time", value, 175u);

    if (t425_memory_read_word(&core.memory, W2 - 16u, &value) != 0)
        fail("ordered W2 TLink read");
    expect_u32("ordered W2 TLink", value, W1 | T425_PRIORITY_LOW);

    if (t425_memory_read_word(&core.memory, W1 - 20u, &value) != 0)
        fail("ordered W1 Time read");
    expect_u32("ordered W1 Time", value, 200u);

    if (t425_memory_read_word(&core.memory, W1 - 16u, &value) != 0)
        fail("ordered W1 TLink read");
    expect_u32("ordered W1 TLink", value, T425_NOT_PROCESS);

    expect_u32("ordered TNext low", core.timer.next_low, 150u);

    printf("TIN future low ordered insertion: PASS\n");
}

static void test_future_high_single(void)
{
    t425_core_t core;
    uint32_t value;
    int rc;

    setup_core(&core, W0, W0 | T425_PRIORITY_HIGH, 100u);
    if (t425_memory_write_word(&core.memory, W1 - 4u, 0x80000080u) != 0) fail("high W1 Iptr write");
    if (t425_memory_write_word(&core.memory, W1 - 8u, T425_NOT_PROCESS) != 0) fail("high W1 Link write");
    core.scheduler.high_head = W1 | T425_PRIORITY_HIGH;
    core.scheduler.high_tail = W1 | T425_PRIORITY_HIGH;
    core.cpu.A = 120u; t425_fixture_sync_slaves(&core.cpu);
    rc = t425_core_step(&core);
    expect_i("high PFIX return", rc, T425_STEP_OK);
    rc = t425_core_step(&core);
    expect_i("high TIN return", rc, T425_STEP_OK);
    expect_u32("high TIN cycles", (uint32_t)core.cycles, 31u);
    if (t425_memory_read_word(&core.memory, T425_TPTR_LOC0, &value) != 0) fail("high TPtrLoc0 read");
    expect_u32("high TPtrLoc0", value, W0 | T425_PRIORITY_HIGH);
    if (t425_memory_read_word(&core.memory, T425_TPTR_LOC1, &value) != 0) fail("high TPtrLoc1 read");
    expect_u32("high TPtrLoc1", value, T425_NOT_PROCESS);
    if (t425_memory_read_word(&core.memory, W0 - 20u, &value) != 0) fail("high Time read");
    expect_u32("high Time", value, 120u);
    if (t425_memory_read_word(&core.memory, W0 - 16u, &value) != 0) fail("high TLink read");
    expect_u32("high TLink", value, T425_NOT_PROCESS);
    if (t425_memory_read_word(&core.memory, W0 - 12u, &value) != 0) fail("high State read");
    expect_u32("high State", value, T425_PROCESS_STATE_WAITING);
    expect_u32("high CPU Wptr", core.cpu.Wptr, W1);
    expect_u32("high current wdesc", core.scheduler.current_wdesc, W1 | T425_PRIORITY_HIGH);
    printf("TIN future high single: PASS\n");
}

int main(void)
{
    test_past();
    test_future_low_single();
    test_future_low_ordered();
    test_future_high_single();

    printf("F08.1-C.4 TIN timer queue tests: PASS\n");
    return 0;
}
