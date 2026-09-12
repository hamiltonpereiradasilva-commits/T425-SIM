#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_clock.h"
#include "t425_timer.h"

static void test_reset(void)
{
    t425_timer_state_t timer = {0};

    t425_timer_reset(&timer);

    assert(timer.high == 0u);
    assert(timer.low == 0u);
    assert(timer.high_cycle_accum == 0u);
    assert(timer.low_cycle_accum == 0u);
}

static void test_high_timer(void)
{
    t425_timer_state_t timer = {0};

    t425_timer_reset(&timer);

    t425_timer_advance(&timer, 19u);
    assert(timer.high == 0u);

    t425_timer_advance(&timer, 1u);
    assert(timer.high == 1u);

    t425_timer_advance(&timer, 40u);
    assert(timer.high == 3u);
}

static void test_low_timer(void)
{
    t425_timer_state_t timer = {0};

    t425_timer_reset(&timer);

    t425_timer_advance(&timer, 1279u);
    assert(timer.low == 0u);

    t425_timer_advance(&timer, 1u);
    assert(timer.low == 1u);

    t425_timer_advance(&timer, 2560u);
    assert(timer.low == 3u);
}

static void test_fractional_progress(void)
{
    t425_timer_state_t timer = {0};

    t425_timer_reset(&timer);

    t425_timer_advance(&timer, 10u);
    assert(timer.high == 0u);
    assert(timer.low == 0u);

    t425_timer_advance(&timer, 10u);
    assert(timer.high == 1u);
    assert(timer.low == 0u);

    assert(timer.high_cycle_accum == 0u);
    assert(timer.low_cycle_accum == 20u);
}

static void test_overflow(void)
{
    t425_timer_state_t timer = {0};

    t425_timer_reset(&timer);

    timer.high = UINT32_MAX;
    timer.low = UINT32_MAX;

    t425_timer_advance(&timer, 20u);

    assert(timer.high == 0u);
    assert(timer.low == UINT32_MAX);
}

int main(void)
{
    assert(T425_CLOCK_HZ == 20000000u);

    test_reset();
    test_high_timer();
    test_low_timer();
    test_fractional_progress();
    test_overflow();

    printf("F08.1-C.4a.1 timer clocks: PASS\n");
    return 0;
}
