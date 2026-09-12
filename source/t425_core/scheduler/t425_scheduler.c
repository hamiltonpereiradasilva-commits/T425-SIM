#include "t425_scheduler.h"

void t425_scheduler_reset(
    t425_scheduler_state_t *scheduler)
{
    if (!scheduler)
        return;

    *scheduler = (t425_scheduler_state_t){0};

    scheduler->high_head = T425_NOT_PROCESS;
    scheduler->high_tail = T425_NOT_PROCESS;
    scheduler->low_head = T425_NOT_PROCESS;
    scheduler->low_tail = T425_NOT_PROCESS;
    scheduler->current_wdesc = T425_NOT_PROCESS;

}
