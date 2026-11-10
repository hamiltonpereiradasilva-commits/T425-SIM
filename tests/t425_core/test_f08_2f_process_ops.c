#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "t425_core.h"
#include "t425_memory.h"
#include "t425_scheduler.h"

static void init_core(t425_core_t *core)
{
    memset(core, 0, sizeof(*core));
    t425_core_reset(core);
}

static void put_opr(t425_core_t *core, uint8_t opr)
{
    uint32_t address = T425_INTERNAL_RAM_BASE;

    core->cpu.IptrReg = address;

    /*
     * Real T425 instruction encoding used by the existing
     * execution tests:
     *
     *   PFIX  = 0x20 | high nibble
     *   OPR   = 0xF0 | low nibble
     */
    assert(t425_memory_write_byte(
        &core->memory,
        address,
        (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu))) == 0);

    assert(t425_memory_write_byte(
        &core->memory,
        address + 1u,
        (uint8_t)(0xF0u | (opr & 0x0Fu))) == 0);
}

static int exec_opr(t425_core_t *core, uint8_t opr)
{
    put_opr(core, opr);

    if (t425_core_step(core) != T425_STEP_OK)
        return T425_STEP_BADMEM;

    return t425_core_step(core);
}

static void write_word(
    t425_core_t *core,
    uint32_t address,
    uint32_t value)
{
    assert(t425_memory_write_word(
        &core->memory, address, value) == 0);
}

static void test_ldpri(void)
{
    t425_core_t core;

    init_core(&core);

    /*
     * Deliberately make architectural WdescReg disagree with the
     * legacy scheduler metadata. LDPRI must use WdescReg.
     */
    core.cpu.WdescReg =
        T425_INTERNAL_RAM_BASE | T425_PRIORITY_LOW;

    core.scheduler.current_wdesc =
        (T425_INTERNAL_RAM_BASE + 0x100u) |
        T425_PRIORITY_HIGH;

    assert(exec_opr(&core, 0x1Eu) == T425_STEP_OK);

    /*
     * LDPRI pushes priority into the evaluation stack.
     */
    assert(core.cpu.A == T425_PRIORITY_LOW);
}

static void test_runp(void)
{
    t425_core_t core;

    const uint32_t wdesc =
        (T425_INTERNAL_RAM_BASE + 0x100u) |
        T425_PRIORITY_LOW;
    const uint32_t wptr =
        wdesc & ~1u;

    init_core(&core);

    /*
     * RUNP reads the process IPTR from Wptr-4.
     */
    write_word(
        &core,
        wptr - 4u,
        T425_INTERNAL_RAM_BASE + 0x300u);

    core.cpu.A = wdesc; t425_fixture_sync_slaves(&core.cpu);

    assert(exec_opr(&core, 0x39u) == T425_STEP_OK);

    assert(
        core.scheduler.fptr[T425_PRIORITY_LOW] == wdesc);

    assert(
        core.scheduler.bptr[T425_PRIORITY_LOW] == wdesc);

    assert(core.cpu.A == 0u);
}

static void test_stopp(void)
{
    t425_core_t core;

    const uint32_t current_wdesc =
        (T425_INTERNAL_RAM_BASE + 0x100u) |
        T425_PRIORITY_HIGH;

    const uint32_t current_wptr =
        current_wdesc & ~1u;

    const uint32_t next_wdesc =
        (T425_INTERNAL_RAM_BASE + 0x200u) |
        T425_PRIORITY_HIGH;

    const uint32_t next_wptr =
        next_wdesc & ~1u;

    const uint32_t next_iptr =
        T425_INTERNAL_RAM_BASE + 0x300u;

    init_core(&core);

    /*
     * B is already ready.
     */
    write_word(
        &core,
        next_wptr - 4u,
        next_iptr);

    assert(t425_scheduler_enqueue(
        &core.scheduler,
        &core.memory,
        next_wdesc) == 0);

    /*
     * A is the running process.
     */
    core.cpu.WdescReg = current_wdesc;
    core.cpu.WptrReg = current_wptr;
    core.cpu.IptrReg =
        T425_INTERNAL_RAM_BASE + 0x400u;

    core.scheduler.current_wdesc = current_wdesc;
    core.scheduler.running = 1u;

    write_word(
        &core,
        current_wptr - 4u,
        core.cpu.IptrReg);

    assert(exec_opr(&core, 0x15u) == T425_STEP_OK);

    /*
     * STOPP saves A and dispatches B through the central
     * scheduler.
     */
    assert(core.cpu.WdescReg == next_wdesc);
    assert(core.cpu.WptrReg == next_wptr);
    assert(core.cpu.IptrReg == next_iptr);
}

static void test_endp(void)
{
    t425_core_t core;

    const uint32_t terminated_wdesc =
        (T425_INTERNAL_RAM_BASE + 0x100u) |
        T425_PRIORITY_HIGH;

    const uint32_t terminated_wptr =
        terminated_wdesc & ~1u;

    const uint32_t successor_wptr =
        T425_INTERNAL_RAM_BASE + 0x200u;

    const uint32_t successor_wdesc =
        successor_wptr | T425_PRIORITY_HIGH;

    const uint32_t successor_iptr =
        T425_INTERNAL_RAM_BASE + 0x300u;

    init_core(&core);

    core.cpu.WdescReg = terminated_wdesc;
    core.cpu.WptrReg = terminated_wptr;
    core.cpu.IptrReg =
        T425_INTERNAL_RAM_BASE + 0x400u;

    core.scheduler.current_wdesc = terminated_wdesc;
    core.scheduler.running = 1u;

    /*
     * ENDP receives successor Wptr in A.
     */
    core.cpu.A = successor_wptr; t425_fixture_sync_slaves(&core.cpu);

    /*
     * Final process in the group.
     */
    write_word(
        &core,
        successor_wptr + 4u,
        1u);

    /*
     * Successor IPTR at Wptr - 4, as defined by the
     * architectural process layout and scheduler dequeue.
     */
    write_word(
        &core,
        successor_wptr - 4u,
        successor_iptr);

    assert(exec_opr(&core, 0x03u) == T425_STEP_OK);

    /*
     * ENDP must dispatch the successor through the scheduler.
     */
    assert(core.cpu.WdescReg == successor_wdesc);
    assert(core.cpu.WptrReg == successor_wptr);
    assert(core.cpu.IptrReg == successor_iptr);

    assert(core.cpu.A == 0u);
    assert(core.cpu.B == 0u);
    assert(core.cpu.C == 0u);
}

int main(void)
{
    test_ldpri();
    test_runp();
    test_stopp();
    test_endp();

    printf("F08.2-F process-operation tests: PASS\n");
    return 0;
}
