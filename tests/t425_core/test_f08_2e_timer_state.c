#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_timer.h"

static void test_reset(void)
{
    t425_timer_state_t timer;

    timer.high = 0x11111111u;
    timer.low = 0x22222222u;
    timer.next_high = 0x33333333u;
    timer.next_low = 0x44444444u;
    timer.interrupt_high_enabled = 1u;
    timer.interrupt_low_enabled = 1u;
    timer.running = 1u;

    t425_timer_reset(&timer);

    assert(timer.high == 0u);
    assert(timer.low == 0u);
    assert(timer.next_high == UINT32_MAX);
    assert(timer.next_low == UINT32_MAX);
    assert(timer.interrupt_high_enabled == 0u);
    assert(timer.interrupt_low_enabled == 0u);
    assert(timer.running == 0u);
}

static void test_independent_clock_state(void)
{
    t425_timer_state_t timer;

    t425_timer_reset(&timer);

    timer.high = 100u;
    timer.low = 200u;
    timer.next_high = 300u;
    timer.next_low = 400u;

    assert(t425_timer_read_high(&timer) == 100u);
    assert(t425_timer_read_low(&timer) == 200u);
    assert(timer.next_high == 300u);
    assert(timer.next_low == 400u);
}

static void test_clock_disabled_until_started(void)
{
    t425_timer_state_t timer;

    t425_timer_reset(&timer);

    t425_timer_advance(&timer, 1000000u);

    assert(timer.high == 0u);
    assert(timer.low == 0u);
}

static void test_clock_progression(void)
{
    t425_timer_state_t timer;

    t425_timer_reset(&timer);
    timer.running = 1u;

    /*
     * Advance enough processor cycles to produce timer ticks.
     * The exact divisors are implementation constants derived
     * from T425_CLOCK_HZ; the invariant tested here is monotonic
     * progression and independent HIGH/LOW counters.
     */
    t425_timer_advance(&timer, 1000000u);

    assert(timer.high > 0u);
    assert(timer.low > 0u);
}

static void test_core_timer_state(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    assert(core.timer.high == 0u);
    assert(core.timer.low == 0u);
    assert(core.timer.next_high == UINT32_MAX);
    assert(core.timer.next_low == UINT32_MAX);
    assert(core.timer.interrupt_high_enabled == 0u);
    assert(core.timer.interrupt_low_enabled == 0u);
    assert(core.timer.running == 0u);
}

int main(void)
{
    test_reset();
    test_independent_clock_state();
    test_clock_disabled_until_started();
    test_clock_progression();
    test_core_timer_state();

    puts("F08.2-E timer state tests: PASS");
    return 0;
}
