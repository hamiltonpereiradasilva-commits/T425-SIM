#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_memory.h"
#include "t425_scheduler.h"

static void test_reset(void)
{
    t425_scheduler_state_t s;

    s.fptr[0] = 0u;
    s.fptr[1] = 0u;
    s.bptr[0] = 0u;
    s.bptr[1] = 0u;
    s.current_wdesc = 0u;
    s.running = 1u;

    t425_scheduler_reset(&s);

    assert(s.fptr[0] == T425_NOT_PROCESS);
    assert(s.fptr[1] == T425_NOT_PROCESS);
    assert(s.bptr[0] == T425_NOT_PROCESS);
    assert(s.bptr[1] == T425_NOT_PROCESS);
    assert(s.current_wdesc == T425_NOT_PROCESS);
    assert(s.running == 0u);
}

static void test_high_fifo(void)
{
    t425_memory_t m;
    t425_scheduler_state_t s;
    uint32_t wdesc;
    uint32_t iptr;

    t425_memory_reset(&m);
    t425_scheduler_reset(&s);

    assert(t425_scheduler_enqueue(&s, &m, 0x80000100u) == 0);
    assert(t425_scheduler_enqueue(&s, &m, 0x80000140u) == 0);
    assert(t425_scheduler_enqueue(&s, &m, 0x80000180u) == 0);

    assert(s.fptr[0] == 0x80000100u);
    assert(s.bptr[0] == 0x80000180u);

    assert(t425_scheduler_dequeue(&s, &m, &wdesc, &iptr) == 0);
    assert(wdesc == 0x80000100u);

    assert(t425_scheduler_dequeue(&s, &m, &wdesc, &iptr) == 0);
    assert(wdesc == 0x80000140u);

    assert(t425_scheduler_dequeue(&s, &m, &wdesc, &iptr) == 0);
    assert(wdesc == 0x80000180u);

    assert(s.fptr[0] == T425_NOT_PROCESS);
}

static void test_priority(void)
{
    t425_memory_t m;
    t425_scheduler_state_t s;
    uint32_t wdesc;
    uint32_t iptr;

    t425_memory_reset(&m);
    t425_scheduler_reset(&s);

    assert(t425_scheduler_enqueue(&s, &m, 0x80000201u) == 0);
    assert(t425_scheduler_enqueue(&s, &m, 0x80000240u) == 0);

    assert(t425_scheduler_dequeue(&s, &m, &wdesc, &iptr) == 0);
    assert(wdesc == 0x80000240u);

    assert(t425_scheduler_dequeue(&s, &m, &wdesc, &iptr) == 0);
    assert(wdesc == 0x80000201u);
}

static void test_priority_isolation(void)
{
    t425_memory_t m;
    t425_scheduler_state_t s;
    uint32_t wdesc;
    uint32_t iptr;

    t425_memory_reset(&m);
    t425_scheduler_reset(&s);

    assert(t425_scheduler_enqueue(&s, &m, 0x80000300u) == 0);
    assert(t425_scheduler_enqueue(&s, &m, 0x80000341u) == 0);

    assert(s.fptr[0] == 0x80000300u);
    assert(s.bptr[0] == 0x80000300u);
    assert(s.fptr[1] == 0x80000341u);
    assert(s.bptr[1] == 0x80000341u);

    assert(t425_scheduler_dequeue(&s, &m, &wdesc, &iptr) == 0);
    assert(wdesc == 0x80000300u);

    assert(s.fptr[1] == 0x80000341u);
    assert(s.bptr[1] == 0x80000341u);
}

int main(void)
{
    test_reset();
    test_high_fifo();
    test_priority();
    test_priority_isolation();

    puts("F08.2-C FPtr/BPtr tests: PASS");
    return 0;
}
