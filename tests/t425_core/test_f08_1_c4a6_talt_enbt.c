#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "t425_core.h"
#include "t425_memory.h"
#include "t425_process.h"

#define W0 0x80000100u

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

static void put_byte(t425_core_t *core, uint32_t address, uint8_t value)
{
    if (t425_memory_write_byte(&core->memory, address, value) != 0)
        fail("program write");
}

static void test_talt(void)
{
    t425_core_t core;
    uint32_t value;

    t425_core_reset(&core);
    core.cpu.Wptr = W0;
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.scheduler.current_wdesc = W0 | T425_PRIORITY_LOW;
    core.scheduler.running = 1u;

    put_byte(&core, T425_INTERNAL_RAM_BASE, 0x24u);
    put_byte(&core, T425_INTERNAL_RAM_BASE + 1u, 0xFEu);

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("TALT PFIX");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("TALT");

    if (t425_memory_read_word(&core.memory, W0 - 12u, &value) != 0)
        fail("TALT state read");

    expect_u32("TALT State", value,
               T425_PROCESS_STATE_ENABLING);

    if (t425_memory_read_word(&core.memory, W0 - 16u, &value) != 0)
        fail("TALT TLink read");

    expect_u32("TALT TLink", value,
               T425_TIMER_TIME_NO_SET);

    expect_u32("TALT cycles", (uint32_t)core.cycles, 5u);
}

static void test_enbt(void)
{
    t425_core_t core;
    uint32_t value;

    t425_core_reset(&core);
    core.cpu.Wptr = W0;
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.scheduler.current_wdesc = W0 | T425_PRIORITY_LOW;
    core.scheduler.running = 1u;

    if (t425_memory_write_word(&core.memory, W0 - 16u,
                               T425_TIMER_TIME_NO_SET) != 0)
        fail("ENBT TLink init");

    core.cpu.A = 1u;
    core.cpu.B = 200u;

    put_byte(&core, T425_INTERNAL_RAM_BASE, 0x24u);
    put_byte(&core, T425_INTERNAL_RAM_BASE + 1u, 0xF7u);

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("ENBT PFIX");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("ENBT");

    if (t425_memory_read_word(&core.memory, W0 - 20u, &value) != 0)
        fail("ENBT Time read");

    expect_u32("ENBT Time", value, 200u);

    if (t425_memory_read_word(&core.memory, W0 - 16u, &value) != 0)
        fail("ENBT TLink read");

    expect_u32("ENBT TLink", value,
               T425_TIMER_TIME_SET);

    expect_u32("ENBT A", core.cpu.A, 1u);
    expect_u32("ENBT cycles", (uint32_t)core.cycles, 9u);
}

static void test_enbt_earlier(void)
{
    t425_core_t core;
    uint32_t value;

    t425_core_reset(&core);
    core.cpu.Wptr = W0;
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.scheduler.current_wdesc = W0 | T425_PRIORITY_LOW;
    core.scheduler.running = 1u;

    if (t425_memory_write_word(&core.memory, W0 - 16u,
                               T425_TIMER_TIME_SET) != 0)
        fail("ENBT earlier TLink init");

    if (t425_memory_write_word(&core.memory, W0 - 20u,
                               200u) != 0)
        fail("ENBT earlier Time init");

    core.cpu.A = 1u;
    core.cpu.B = 150u;

    put_byte(&core, T425_INTERNAL_RAM_BASE, 0x24u);
    put_byte(&core, T425_INTERNAL_RAM_BASE + 1u, 0xF7u);

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("ENBT earlier PFIX");

    if (t425_core_step(&core) != T425_STEP_OK)
        fail("ENBT earlier");

    if (t425_memory_read_word(&core.memory, W0 - 20u, &value) != 0)
        fail("ENBT earlier Time read");

    expect_u32("ENBT earlier Time", value, 150u);

    if (t425_memory_read_word(&core.memory, W0 - 16u, &value) != 0)
        fail("ENBT earlier TLink read");

    expect_u32("ENBT earlier TLink", value,
               T425_TIMER_TIME_SET);
}

int main(void)
{
    test_talt();
    printf("C4-A.6 TALT: PASS\n");

    test_enbt();
    printf("C4-A.7 ENBT first timer: PASS\n");

    test_enbt_earlier();
    printf("C4-A.7 ENBT earlier timer: PASS\n");

    printf("F08.1-C.4.6/7 ALT timer setup: PASS\n");
    return 0;
}
