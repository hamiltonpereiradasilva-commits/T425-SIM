#include "t425_timer.h"

#define T425_TIMER_HIGH_TICK_CYCLES (T425_CLOCK_HZ / 1000000u)
#define T425_TIMER_LOW_TICK_CYCLES  (T425_CLOCK_HZ / 15625u)

_Static_assert(T425_TIMER_HIGH_TICK_CYCLES != 0u,
               "T425 high timer tick is below one processor cycle");

_Static_assert(T425_TIMER_LOW_TICK_CYCLES != 0u,
               "T425 low timer tick is below one processor cycle");

void t425_timer_reset(t425_timer_state_t *timer)
{
    if (!timer)
        return;

    *timer = (t425_timer_state_t){0};
    timer->next_high = UINT32_MAX;
    timer->next_low = UINT32_MAX;
    /* T425 hardware clocks tick periodically from reset. */
    timer->running = 1u;
}

void t425_timer_advance(
    t425_timer_state_t *timer,
    uint32_t processor_cycles)
{
    uint32_t high_ticks;
    uint32_t low_ticks;

    if (!timer || processor_cycles == 0u || !timer->running)
        return;

    timer->high_cycle_accum += processor_cycles;
    timer->low_cycle_accum += processor_cycles;

    high_ticks =
        timer->high_cycle_accum / T425_TIMER_HIGH_TICK_CYCLES;

    low_ticks =
        timer->low_cycle_accum / T425_TIMER_LOW_TICK_CYCLES;

    timer->high_cycle_accum %=
        T425_TIMER_HIGH_TICK_CYCLES;

    timer->low_cycle_accum %=
        T425_TIMER_LOW_TICK_CYCLES;

    timer->high += high_ticks;
    timer->low += low_ticks;
}

uint32_t t425_timer_read_high(
    const t425_timer_state_t *timer)
{
    return timer ? timer->high : 0u;
}

uint32_t t425_timer_read_low(
    const t425_timer_state_t *timer)
{
    return timer ? timer->low : 0u;
}
