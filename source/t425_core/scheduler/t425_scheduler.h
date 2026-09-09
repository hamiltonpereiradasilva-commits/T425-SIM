#ifndef T425_CORE_SCHEDULER_H
#define T425_CORE_SCHEDULER_H

#include <stdint.h>

typedef struct
{
    uint32_t high_head;
    uint32_t high_tail;

    uint32_t low_head;
    uint32_t low_tail;

    uint8_t running;
} t425_scheduler_state_t;

void t425_scheduler_reset(
    t425_scheduler_state_t *scheduler);

#endif
