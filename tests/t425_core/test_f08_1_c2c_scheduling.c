#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "t425_core.h"
#include "t425_process.h"

static void put_opr(t425_core_t *core, uint32_t opr)
{
    uint32_t a = T425_INTERNAL_RAM_BASE;
    core->cpu.Iptr = a;
    assert(t425_memory_write_byte(&core->memory, a,
        (uint8_t)(0x20u | ((opr >> 4) & 0x0Fu))) == 0);
    assert(t425_memory_write_byte(&core->memory, a + 1u,
        (uint8_t)(0xF0u | (opr & 0x0Fu))) == 0);
}

static void exec_opr(t425_core_t *core, uint32_t opr)
{
    put_opr(core, opr);
    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static void test_ldpri(void)
{
    t425_core_t c;
    t425_core_reset(&c);

    c.scheduler.current_wdesc =
        T425_INTERNAL_RAM_BASE + T425_PRIORITY_HIGH;
    exec_opr(&c, 0x1Eu);
    assert(c.cpu.A == 0u);

    t425_core_reset(&c);
    c.scheduler.current_wdesc =
        T425_INTERNAL_RAM_BASE + T425_PRIORITY_LOW;
    exec_opr(&c, 0x1Eu);
    assert(c.cpu.A == 1u);
}

static void test_runp(void)
{
    t425_core_t c;
    uint32_t wptr = T425_INTERNAL_RAM_BASE + 0x200u;
    uint32_t iptr = T425_INTERNAL_RAM_BASE + 0x300u;
    uint32_t wdesc = wptr | T425_PRIORITY_LOW;

    t425_core_reset(&c);
    c.cpu.A = wdesc;
    assert(t425_memory_write_word(&c.memory, wptr - 4u, iptr) == 0);

    exec_opr(&c, 0x39u);

    assert(c.scheduler.low_head == wdesc);
    assert(c.scheduler.low_tail == wdesc);
    assert(c.cpu.A == 0u);
    assert(c.cycles == 11u);
}

static void test_stopp(void)
{
    t425_core_t c;
    uint32_t current = T425_INTERNAL_RAM_BASE + 0x100u;
    uint32_t next = T425_INTERNAL_RAM_BASE + 0x200u;
    uint32_t next_iptr = T425_INTERNAL_RAM_BASE + 0x300u;

    t425_core_reset(&c);
    c.cpu.Wptr = current;
    c.scheduler.current_wdesc = current;
    c.scheduler.running = 1u;
    c.scheduler.high_head = next;
    c.scheduler.high_tail = next;

    assert(t425_memory_write_word(&c.memory, next - 4u, next_iptr) == 0);
    assert(t425_memory_write_word(&c.memory, next - 8u,
                                  T425_NOT_PROCESS) == 0);

    exec_opr(&c, 0x15u);

    assert(c.cpu.Wptr == next);
    assert(c.cpu.Iptr == next_iptr);
    assert(c.scheduler.current_wdesc == next);
    assert(c.scheduler.high_head == T425_NOT_PROCESS);
    assert(c.cycles == 12u);
}

static void test_startp(void)
{
    t425_core_t c;
    uint32_t parent = T425_INTERNAL_RAM_BASE + 0x100u;
    uint32_t child = T425_INTERNAL_RAM_BASE + 0x200u;
    uint32_t child_iptr = T425_INTERNAL_RAM_BASE + 0x300u;
    uint32_t child_wdesc = child | T425_PRIORITY_HIGH;
    uint32_t instruction = T425_INTERNAL_RAM_BASE;

    t425_core_reset(&c);

    c.cpu.Wptr = parent;
    c.cpu.A = child;
    c.scheduler.current_wdesc = parent | T425_PRIORITY_HIGH;
    c.scheduler.running = 1u;

    /*
     * put_opr() installs PFIX + STARTP at 'instruction'.
     * At STARTP execution Iptr has advanced past the two bytes.
     */
    c.cpu.B = child_iptr - (instruction + 2u);

    put_opr(&c, 0x0Du);

    assert(t425_core_step(&c) == T425_STEP_OK);
    assert(t425_core_step(&c) == T425_STEP_OK);

    assert(c.cpu.Wptr == parent);
    assert(c.scheduler.current_wdesc == (parent | T425_PRIORITY_HIGH));
    assert(c.scheduler.high_head == child_wdesc);
    assert(c.scheduler.high_tail == child_wdesc);

    {
        uint32_t stored_iptr;

        assert(t425_memory_read_word(
            &c.memory, child - 4u, &stored_iptr) == 0);

        assert(stored_iptr == child_iptr);
    }

    assert(c.cycles == 13u);
}

static void test_endp(void)
{
    t425_core_t c;
    uint32_t succ = T425_INTERNAL_RAM_BASE + 0x200u;
    uint32_t succ_iptr = T425_INTERNAL_RAM_BASE + 0x300u;

    t425_core_reset(&c);
    c.cpu.A = succ;
    c.scheduler.current_wdesc = T425_INTERNAL_RAM_BASE + 0x100u;
    assert(t425_memory_write_word(&c.memory, succ, succ_iptr) == 0);
    assert(t425_memory_write_word(&c.memory, succ + 4u, 1u) == 0);

    exec_opr(&c, 0x03u);

    assert(c.cpu.Wptr == succ);
    assert(c.cpu.Iptr == succ_iptr);
    assert(c.scheduler.current_wdesc == succ);
    assert(c.cycles == 14u);
}

int main(void)
{
    test_ldpri();
    test_runp();
    test_stopp();
    test_startp();
    test_endp();
    puts("F08.1-C.2c scheduling: PASS");
    return 0;
}
