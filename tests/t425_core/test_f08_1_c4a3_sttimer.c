#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

static void put_program(t425_core_t *core)
{
    assert(t425_memory_write_byte(
        &core->memory, T425_INTERNAL_RAM_BASE + 0u, 0x25u) == 0);
    assert(t425_memory_write_byte(
        &core->memory, T425_INTERNAL_RAM_BASE + 1u, 0xF4u) == 0);
}

int main(void)
{
    t425_core_t core = {0};

    t425_core_reset(&core);

    core.cpu.Iptr = T425_INTERNAL_RAM_BASE;
    core.cpu.A = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);

    core.timer.high = 0xAAAAAAAAu;
    core.timer.low  = 0x55555555u;

    core.scheduler.current_wdesc = 0x00000100u;
    core.scheduler.running = 1u;

    put_program(&core);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.timer.high == 0x12345678u);
    assert(core.timer.low  == 0x12345678u);
    assert(core.cycles == 2u);

    printf("F08.1-C.4a.3 STTIMER: PASS\n");
    return 0;
}
