#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_process.h"

static void put_opr(t425_core_t *core, uint32_t opr)
{
    uint32_t address = T425_INTERNAL_RAM_BASE;

    core->cpu.Iptr = address;

    assert(t425_memory_write_byte(
        &core->memory,
        address,
        (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu))) == 0);

    assert(t425_memory_write_byte(
        &core->memory,
        address + 1u,
        (uint8_t)(0xF0u | (opr & 0x0Fu))) == 0);
}

static void test_stoperr_without_error(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.scheduler.running = 1u;
    core.scheduler.current_wdesc =
        T425_INTERNAL_RAM_BASE + 0x100u;

    put_opr(&core, 0x55u);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(core.cpu.error == 0u);
    assert(core.cpu.halted == 0u);
    assert(core.scheduler.running == 1u);
    assert(core.cycles == 3u);
}

static void test_stoperr_selects_high_priority(void)
{
    t425_core_t core;

    const uint32_t current_wptr =
        T425_INTERNAL_RAM_BASE + 0x100u;
    const uint32_t next_wptr =
        T425_INTERNAL_RAM_BASE + 0x200u;
    const uint32_t next_iptr =
        T425_INTERNAL_RAM_BASE + 0x300u;

    t425_core_reset(&core);

    core.cpu.error = 1u;
    core.cpu.Wptr = current_wptr;

    core.scheduler.running = 1u;
    core.scheduler.current_wdesc = current_wptr;

    core.scheduler.high_head = next_wptr;
    core.scheduler.high_tail = next_wptr;

    assert(t425_memory_write_word(
        &core.memory,
        next_wptr - 4u,
        next_iptr) == 0);

    assert(t425_memory_write_word(
        &core.memory,
        next_wptr - 8u,
        T425_NOT_PROCESS) == 0);

    put_opr(&core, 0x55u);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.error == 1u);
    assert(core.cpu.halted == 0u);

    assert(core.scheduler.running == 1u);
    assert(core.scheduler.current_wdesc == next_wptr);

    assert(core.cpu.Wptr == next_wptr);
    assert(core.cpu.Iptr == next_iptr);

    assert(core.scheduler.high_head == T425_NOT_PROCESS);
    assert(core.scheduler.high_tail == T425_NOT_PROCESS);

    assert(core.cycles == 3u);
}

static void test_stoperr_no_next_process(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    core.cpu.error = 1u;
    core.cpu.Wptr =
        T425_INTERNAL_RAM_BASE + 0x100u;

    core.scheduler.running = 1u;
    core.scheduler.current_wdesc = core.cpu.Wptr;

    put_opr(&core, 0x55u);

    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);

    assert(core.cpu.error == 1u);
    assert(core.cpu.halted == 0u);

    assert(core.scheduler.running == 0u);
    assert(core.scheduler.current_wdesc == T425_NOT_PROCESS);

    assert(core.cycles == 3u);
}

int main(void)
{
    test_stoperr_without_error();
    test_stoperr_selects_high_priority();
    test_stoperr_no_next_process();

    puts("F08.1-C.2b STOPERR: PASS");
    return 0;
}
