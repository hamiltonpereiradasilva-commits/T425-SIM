#include "t425_core.h"

void t425_core_reset(t425_core_t *core)
{
    if (!core)
        return;

    t425_cpu_reset(&core->cpu);
    t425_memory_reset(&core->memory);
    t425_timer_reset(&core->timer);

    core->scheduler.high_head = T425_NOT_PROCESS;
    core->scheduler.high_tail = T425_NOT_PROCESS;
    core->scheduler.low_head = T425_NOT_PROCESS;
    core->scheduler.low_tail = T425_NOT_PROCESS;
    core->scheduler.current_wdesc = T425_NOT_PROCESS;
    core->scheduler.running = 0u;

    core->cycles = 0;
    core->instructions = 0;
}
