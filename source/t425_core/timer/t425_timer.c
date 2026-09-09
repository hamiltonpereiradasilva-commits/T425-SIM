#include "t425_timer.h"

void t425_timer_reset(
    t425_timer_state_t *timer)
{
    if (!timer)
        return;

    *timer = (t425_timer_state_t){0};

    timer->enabled_high = 1;
    timer->enabled_low = 1;
}
