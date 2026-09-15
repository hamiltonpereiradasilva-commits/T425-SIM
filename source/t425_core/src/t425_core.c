#include "t425_core.h"

void t425_core_reset(t425_core_t *core)
{
    if (!core)
        return;

    t425_cpu_reset(&core->cpu);
    t425_memory_reset(&core->memory);
    t425_timer_reset(&core->timer);
    t425_scheduler_reset(&core->scheduler);core->t425_channel_select = 0u;
    core->t425_channel_stack = 0u;
    core->t425_data_mem_addr = 0u;
    core->t425_data_reg = 0u;

    {
        uint32_t i;
        for (i = 0u; i < T425_MAX_PROCESS_CONTEXTS; ++i) {
            core->process_contexts[i].valid = 0u;
            core->process_contexts[i].wptr = 0u;
            core->process_contexts[i].Wdesc = 0u;
            core->process_contexts[i].Iptr = 0u;
            core->process_contexts[i].A = 0u;
            core->process_contexts[i].B = 0u;
            core->process_contexts[i].C = 0u;
            core->process_contexts[i].Ereg = 0u;
            core->process_contexts[i].Status = 0u;
        }

        for (i = 0u; i < T425_MAX_INTERRUPT_CONTEXTS; ++i) {
            core->interrupt_contexts[i].valid = 0u;
            core->interrupt_contexts[i].WdescIntSave = 0u;
            core->interrupt_contexts[i].IptrIntSave = 0u;
            core->interrupt_contexts[i].AregSave = 0u;
            core->interrupt_contexts[i].BregSave = 0u;
            core->interrupt_contexts[i].CregSave = 0u;
            core->interrupt_contexts[i].StatusIntSave = 0u;
            core->interrupt_contexts[i].EregIntSave = 0u;
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
