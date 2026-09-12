#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_instruction.h"

static void put_program(t425_core_t *core)
{
    assert(t425_memory_write_byte(&core->memory, T425_INTERNAL_RAM_BASE + 0u, 0x22u) == 0);
    assert(t425_memory_write_byte(&core->memory, T425_INTERNAL_RAM_BASE + 1u, 0xF2u) == 0);
}

static void test_high_priority_ldtimer(void)
{
    t425_core_t core = {0};

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    core.scheduler.current_wdesc = 0x00000100u;
    core.scheduler.running = 1u;

    core.timer.high = 0x12345678u;
    core.timer.low = 0x87654321u;

    put_program(&core);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 0x12345678u);
    assert(core.cpu.B == 0u);
    assert(core.cycles == 3u);
}

static void test_low_priority_ldtimer(void)
{
    t425_core_t core = {0};

    t425_core_reset(&core);
    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;

    core.scheduler.current_wdesc = 0x00000101u;
    core.scheduler.running = 1u;

    core.timer.high = 0x12345678u;
    core.timer.low = 0x87654321u;

    put_program(&core);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.A == 0x87654321u);
    assert(core.cpu.B == 0u);
    assert(core.cycles == 3u);
}

int main(void)
{
    test_high_priority_ldtimer();
    test_low_priority_ldtimer();

    printf("F08.1-C.4a.2 LDTIMER priority tests: PASS\n");
    return 0;
}
