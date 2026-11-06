#include "t425_core.h"

static void publish_rx_admission(t425_core_t *core)
{
    if (core == 0 || core->phy == 0)
        return;

    if (core->phy->ops.set_rx_arbitration != 0)
        (void)t425_core_phy_set_rx_arbitration(
            core->phy, core->boot_state == T425_BOOT_WAIT_CONTROL);

    for (uint32_t endpoint = 0u;
         endpoint < T425_CORE_PHY_MAX_ENDPOINTS;
         ++endpoint) {
        bool enabled = false;
        bool issue = false;

        if (core->boot_state == T425_BOOT_WAIT_CONTROL) {
            enabled = true;
        } else if (core->boot_state == T425_BOOT_POKE_ADDRESS ||
                   core->boot_state == T425_BOOT_POKE_DATA ||
                   core->boot_state == T425_BOOT_PEEK_ADDRESS ||
                   core->boot_state == T425_BOOT_PEEK_OUTPUT ||
                   core->boot_state == T425_BOOT_LOAD) {
            enabled = endpoint == core->bootstrap_link;
        } else if (core->boot_state == T425_BOOT_RUNNING) {
            enabled = t425_core_link_rx_admission(core, endpoint);
        }

        if (enabled != (core->rx_credit_enabled[endpoint] != 0u) ||
            (enabled && core->rx_credit_rearm[endpoint] != 0u))
            issue = true;

        if (issue) {
            core->rx_credit_generation[endpoint] =
                (core->rx_credit_generation[endpoint] + 1u) & 0x7FFFFFFFu;
            if (core->rx_credit_generation[endpoint] == 0u)
                core->rx_credit_generation[endpoint] = 1u;
            core->rx_credit_enabled[endpoint] = enabled ? 1u : 0u;
            core->rx_credit_rearm[endpoint] = 0u;
            (void)t425_core_phy_set_rx_admission_credit(
                core->phy, endpoint,
                core->rx_credit_generation[endpoint], enabled);
        }
    }
}

static bool system_inputs(
    t425_core_t *core,
    t425_core_phy_system_inputs_t *inputs)
{
    if (core == 0 || inputs == 0 || core->phy == 0 ||
        core->phy->ops.read_system_inputs == 0)
        return false;
    return t425_core_phy_read_system_inputs(core->phy, inputs);
}

static void architectural_reset(t425_core_t *core)
{
    t425_core_phy_t *phy = core->phy;

    t425_cpu_reset(&core->cpu);
    t425_microsequencer_reset(&core->microsequencer, 0u);
    core->microentry_signal = 0u;
    core->step_instruction = 0u;
    core->next_step_instruction = 0u;
    t425_datapath_reset(&core->datapath);
    t425_timer_reset(&core->timer);
    t425_scheduler_reset(&core->scheduler);

    /*
     * These context slots are emulator bookkeeping, not architectural
     * T425 RAM. Architectural Reset invalidates them while preserving
     * core->memory.
     */
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
    }

    core->t425_channel_select = 0u;
    core->t425_channel_stack = 0u;
    t425_channel_dma_bank_init(&core->t425_channel_dma);
    core->t425_data_mem_addr = 0u;
    core->experimental_data_mem_addr_from_wdesc = 0u;
    core->t425_data_reg = 0u;
    core->previous_read_data = 0u;
    core->data_in = 0u;
    core->data_in_reg = 0u;
    core->two_reads_latch = 0u;
    core->clear_bwse_latch_after_write = 0u;
    core->byte_write_strobe_latch = 0u;
    core->byte_write_pending = 0u;
    core->byte_write_pending_mask = 0u;
    core->byte_write_pending_clear_latch = 0u;
    core->byte_write_pending_address = 0u;
    core->byte_write_pending_data = 0u;
    core->byte_write_pending_iptr = 0u;
    core->byte_write_pending_mpc = 0u;
    core->t425_channel_data_reg = 0u;
    core->clock_reg0 = 0u;
    core->clock_reg1 = 0u;
    core->t425_channel_ready_not_run = 0u;
    core->t425_channel_run_req = 0u;
    core->t425_channel_status = 0u;
    core->t425_alt_enabled = 0u;
    core->phy = phy;
    core->bootfromrom_sampled = 0u;
    core->bootfromrom_value = 0u;
    core->bootstrap_link = T425_CORE_PHY_MAX_ENDPOINTS;
    core->bootstrap_count = 0u;
    core->bootstrap_index = 0u;
    core->bootstrap_address = 0u;
    core->bootstrap_word = 0u;
    core->bootstrap_byte_index = 0u;
    for (uint32_t i = 0u; i < T425_CORE_PHY_MAX_ENDPOINTS; ++i) {
        core->rx_credit_generation[i] = 0u;
        core->rx_credit_enabled[i] = 0u;
        core->rx_credit_rearm[i] = 0u;
    }
    core->boot_state = T425_BOOT_DECIDE;
    core->scheduler.running = 0u;
}

static bool receive_byte(t425_core_t *core, uint32_t link, uint8_t *byte)
{
    return t425_core_link_input_available(core, link) &&
           t425_core_link_input_consume(core, link, byte) == 0;
}

static void finish_transaction(t425_core_t *core)
{
    core->bootstrap_link = T425_CORE_PHY_MAX_ENDPOINTS;
    core->bootstrap_byte_index = 0u;
    core->bootstrap_word = 0u;
    core->boot_state = T425_BOOT_WAIT_CONTROL;
}

static void enter_running(t425_core_t *core)
{
    core->boot_state = T425_BOOT_RUNNING;
    core->scheduler.running = 1u;
    core->scheduler.current_wdesc = core->cpu.WdescReg;
}

static void enter_low_priority_process(
    t425_core_t *core,
    uint32_t iptr,
    uint32_t wptr)
{
    core->cpu.IptrReg = iptr;
    /* Wdesc is the authoritative descriptor, including priority bits. */
    core->cpu.WdescReg = wptr | T425_PRIORITY_LOW;
    enter_running(core);
}

static bool bootstrap_step(t425_core_t *core)
{
    uint8_t byte;
    uint32_t link;

    switch (core->boot_state) {
    case T425_BOOT_DECIDE:
        if (core->bootfromrom_value) {
            enter_low_priority_process(
                core, T425_ROM_ENTRY, T425_MEM_START);
        } else {
            core->boot_state = T425_BOOT_WAIT_CONTROL;
        }
        return true;

    case T425_BOOT_WAIT_CONTROL:
        for (link = 0u; link < T425_CORE_PHY_MAX_ENDPOINTS; ++link) {
            if (!receive_byte(core, link, &byte))
                continue;
            core->bootstrap_link = (uint8_t)link;
            core->bootstrap_byte_index = 0u;
            core->bootstrap_word = 0u;
            if (byte == 0u) {
                core->boot_state = T425_BOOT_POKE_ADDRESS;
            } else if (byte == 1u) {
                core->boot_state = T425_BOOT_PEEK_ADDRESS;
            } else {
                core->bootstrap_count = byte;
                core->bootstrap_index = 0u;
                core->boot_state = T425_BOOT_LOAD;
            }
            return true;
        }
        return true;

    case T425_BOOT_POKE_ADDRESS:
        if (!receive_byte(core, core->bootstrap_link, &byte))
            return true;
        core->bootstrap_word |=
            (uint32_t)byte << (8u * core->bootstrap_byte_index);
        ++core->bootstrap_byte_index;
        if (core->bootstrap_byte_index == 4u) {
            core->bootstrap_address = core->bootstrap_word;
            core->bootstrap_word = 0u;
            core->bootstrap_byte_index = 0u;
            core->boot_state = T425_BOOT_POKE_DATA;
        }
        return true;

    case T425_BOOT_POKE_DATA:
        if (!receive_byte(core, core->bootstrap_link, &byte))
            return true;
        core->bootstrap_word |=
            (uint32_t)byte << (8u * core->bootstrap_byte_index);
        ++core->bootstrap_byte_index;
        if (core->bootstrap_byte_index == 4u) {
            if (t425_memory_write_word(
                    &core->memory,
                    core->bootstrap_address,
                    core->bootstrap_word) != 0)
                core->cpu.error = 1u;
            finish_transaction(core);
        }
        return true;

    case T425_BOOT_PEEK_ADDRESS:
        if (!receive_byte(core, core->bootstrap_link, &byte))
            return true;
        core->bootstrap_word |=
            (uint32_t)byte << (8u * core->bootstrap_byte_index);
        ++core->bootstrap_byte_index;
        if (core->bootstrap_byte_index == 4u) {
            core->bootstrap_address = core->bootstrap_word;
            core->bootstrap_word = 0u;
            core->bootstrap_byte_index = 0u;
            if (t425_memory_read_word(
                    &core->memory,
                    core->bootstrap_address,
                    &core->bootstrap_word) != 0) {
                core->cpu.error = 1u;
                finish_transaction(core);
            } else {
                core->boot_state = T425_BOOT_PEEK_OUTPUT;
            }
        }
        return true;

    case T425_BOOT_PEEK_OUTPUT:
        core->t425_channel_data_reg =
            (core->bootstrap_word >>
             (8u * core->bootstrap_byte_index)) & 0xFFu;
        if (t425_core_link_output(
                core, core->bootstrap_link) == 0) {
            ++core->bootstrap_byte_index;
            if (core->bootstrap_byte_index == 4u)
                finish_transaction(core);
        }
        return true;

    case T425_BOOT_LOAD:
        if (!receive_byte(core, core->bootstrap_link, &byte))
            return true;
        if (t425_memory_write_byte(
                &core->memory,
                T425_MEM_START + core->bootstrap_index,
                byte) != 0) {
            core->cpu.error = 1u;
            finish_transaction(core);
            return true;
        }
        ++core->bootstrap_index;
        if (core->bootstrap_index == core->bootstrap_count) {
            uint32_t wptr =
                (T425_MEM_START + core->bootstrap_count + 3u) & ~3u;
            enter_low_priority_process(core, T425_MEM_START, wptr);
            core->cpu.C = 0x80000010u +
                          ((uint32_t)core->bootstrap_link * 4u);
        }
        return true;

    case T425_BOOT_RUNNING:
    case T425_BOOT_RESET:
    default:
        return false;
    }
}

bool t425_core_system_step(t425_core_t *core)
{
    t425_core_phy_system_inputs_t inputs;

    if (!system_inputs(core, &inputs))
        return false;

    if (inputs.reset) {
        core->reset_active = 1u;
        core->boot_state = T425_BOOT_RESET;
        core->bootfromrom_sampled = 0u;
        core->bootstrap_link = T425_CORE_PHY_MAX_ENDPOINTS;
        publish_rx_admission(core);
        return true;
    }

    if (core->reset_active) {
        architectural_reset(core);
        core->reset_active = 0u;
        core->bootfromrom_value = inputs.boot_from_rom;
        core->bootfromrom_sampled = 1u;
        publish_rx_admission(core);
        return true;
    }

    if (core->boot_state != T425_BOOT_RUNNING) {
        bool consumed = bootstrap_step(core);
        publish_rx_admission(core);
        return consumed;
    }

    publish_rx_admission(core);
    return false;
}
