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

    core->t425_channel_select = 0u;
    core->t425_channel_stack = 0u;
    core->t425_data_mem_addr = 0u;
    core->t425_data_reg = 0u;

    {
        uint32_t i;
        for (i = 0u; i < T425_MAX_PROCESS_CONTEXTS; ++i) {
            core->process_contexts[i].valid = 0u;
            core->process_contexts[i].wptr = 0u;
            core->process_contexts[i].A = 0u;
            core->process_contexts[i].B = 0u;
            core->process_contexts[i].C = 0u;
        }

        for (i = 0u; i < T425_MAX_CHANNEL_RENDEZVOUS; ++i) {
            core->channel_rendezvous[i].valid = 0u;
            core->channel_rendezvous[i].operation = T425_RENDEZVOUS_NONE;
            core->channel_rendezvous[i].wdesc = 0u;
            core->channel_rendezvous[i].wptr = 0u;
            core->channel_rendezvous[i].channel_address = 0u;
            core->channel_rendezvous[i].data_address = 0u;
            core->channel_rendezvous[i].data_length = 0u;
        }
    }

    core->cycles = 0;
    core->instructions = 0;
}
