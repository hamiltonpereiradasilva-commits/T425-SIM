#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_memory.h"

#define CODE_BASE T425_INTERNAL_RAM_BASE

static void reset_core(t425_core_t *core)
{
    t425_core_reset(core);
    core->cpu.Iptr = CODE_BASE;
}

static void emit_op(t425_core_t *core, uint16_t code)
{
    uint8_t pfix;
    uint8_t opr;

    assert(code <= 0xFFu);

    pfix = (uint8_t)(0x20u | ((code >> 4u) & 0x0Fu));
    opr = (uint8_t)(0xF0u | (code & 0x0Fu));

    assert(t425_memory_write_byte(&core->memory,
                                  CODE_BASE,
                                  pfix) == 0);
    assert(t425_memory_write_byte(&core->memory,
                                  CODE_BASE + 1u,
                                  opr) == 0);

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static void test_lb(void)
{
    t425_core_t core;
    reset_core(&core);

    const uint32_t addr = CODE_BASE + 0x100u;
    assert(t425_memory_write_byte(&core.memory, addr, 0xA5u) == 0);

    core.cpu.A = addr; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x11223344u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x55667788u; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x01u);

    assert(core.cpu.A == 0xA5u);
    assert(core.cpu.B == 0x11223344u);
    assert(core.cpu.C == 0x55667788u);
}

static void test_bsub_wsub_wsubdb(void)
{
    t425_core_t core;

    reset_core(&core);
    core.cpu.A = 0x80000100u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 7u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x02u);
    assert(core.cpu.A == 0x80000107u);
    assert(core.cpu.B == 0x12345678u);

    reset_core(&core);
    core.cpu.A = 0x80000100u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 7u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x0Au);
    assert(core.cpu.A == 0x8000011Cu);
    assert(core.cpu.B == 0x12345678u);

    reset_core(&core);
    core.cpu.A = 0x80000100u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 7u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x81u);
    assert(core.cpu.A == 0x80000138u);
    assert(core.cpu.B == 0x12345678u);
}

static void test_sb(void)
{
    t425_core_t core;
    reset_core(&core);

    const uint32_t addr = CODE_BASE + 0x120u;

    core.cpu.A = addr; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0xDEADBEEFu; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0xCAFEBABEu; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x3Bu);

    uint8_t value = 0;
    assert(t425_memory_read_byte(&core.memory, addr, &value) == 0);
    assert(value == 0xEFu);
    assert(core.cpu.A == 0xCAFEBABEu);
}

static void test_bcnt(void)
{
    t425_core_t core;
    reset_core(&core);

    core.cpu.A = 123u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x11111111u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x22222222u; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x34u);

    assert(core.cpu.A == 492u);
    assert(core.cpu.B == 0x11111111u);
    assert(core.cpu.C == 0x22222222u);
}

static void test_wcnt(void)
{
    t425_core_t core;
    reset_core(&core);

    core.cpu.A = 0x80000107u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x11223344u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x55667788u; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x3Fu);

    assert(core.cpu.A == 0x20000041u);
    assert(core.cpu.B == 3u);
    assert(core.cpu.C == 0x11223344u);
}

static void test_scheduler_store(void)
{
    t425_core_t core;
    reset_core(&core);

    core.cpu.A = 0x11111110u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x18u);
    assert(core.scheduler.high_head == 0x11111110u);

    reset_core(&core);
    core.cpu.A = 0x22222220u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x50u);
    assert(core.scheduler.high_tail == 0x22222220u);

    reset_core(&core);
    core.cpu.A = 0x33333330u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x1Cu);
    assert(core.scheduler.low_head == 0x33333330u);

    reset_core(&core);
    core.cpu.A = 0x44444440u; t425_fixture_sync_slaves(&core.cpu);
    emit_op(&core, 0x17u);
    assert(core.scheduler.low_tail == 0x44444440u);
}

int main(void)
{
    test_lb();
    test_bsub_wsub_wsubdb();
    test_sb();
    test_bcnt();
    test_wcnt();
    test_scheduler_store();

    printf("F08.1-C.6.1 workspace/index operations: PASS\n");
    return 0;
}
