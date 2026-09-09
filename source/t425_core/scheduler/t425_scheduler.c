#include "t425_scheduler.h"

void t425_scheduler_reset(
    t425_scheduler_state_t *scheduler)
{
    if (!scheduler)
        return;

    *scheduler = (t425_scheduler_state_t){0};
}
