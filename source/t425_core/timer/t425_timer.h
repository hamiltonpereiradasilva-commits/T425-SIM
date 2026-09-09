#ifndef T425_CORE_TIMER_H
#define T425_CORE_TIMER_H

#include <stdint.h>

typedef struct
{
    uint32_t high;
    uint32_t low;

    uint8_t enabled_high;
    uint8_t enabled_low;
} t425_timer_state_t;

void t425_timer_reset(
    t425_timer_state_t *timer);

#endif
