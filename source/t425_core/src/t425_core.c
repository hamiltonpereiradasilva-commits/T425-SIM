#include "t425_core.h"
#include "t425_channel.h"
#include "t425_mir_fields.h"
#include "t425_byte_write.h"
#include "t425_channel_dma_handoff_28.h"
#include "t425_channel_compat_rx_adapter_07.h"
#include "t425_channel_route_config_52_table.h"
#include "t425_core_ifnext_34.h"
#include "t425_prevread_forwarding_87.h"
#include <string.h>
#include <stdio.h>

static const t425_channel_route_52_t *channel_route_52_find_28(
    uint32_t address);

/* F250/F252: live unique-owner arbitration at the architectural IF Next boundary. */
typedef struct {
    uint8_t lane;
    uint8_t priority_high;
    t425_channel_compat_input_key_t key;
} f250_ifnext_candidate_t;

static bool f250_same_key(const t425_channel_compat_input_key_t *a,
                          const t425_channel_compat_input_key_t *b)
{
    return a->request_id == b->request_id && a->generation == b->generation &&
        a->route.architectural_channel == b->route.architectural_channel &&
        a->route.input_lane == b->route.input_lane &&
        a->route.stack_lane == b->route.stack_lane &&
        a->route.shared_index == b->route.shared_index &&
        a->route.endpoint_id == b->route.endpoint_id &&
        a->route.physical_link == b->route.physical_link &&
        a->route.lane_id == b->route.lane_id;
}

static bool f250_ifnext_unique_input(
    const t425_channel_compat_input_t *input,
    f250_ifnext_candidate_t *candidate)
{
    t425_channel_ifnext_owner_result_t status;
    if (input == NULL || candidate == NULL) return false;
    memset(candidate, 0, sizeof(*candidate));
    status = t425_channel_compat_input_preview_unique_ifnext_owner(
        input, true, &candidate->key);
    if (status != T425_CHANNEL_IFNEXT_OWNER_SELECTED) return false;
    candidate->lane = candidate->key.route.input_lane;
    candidate->priority_high = (uint8_t)(
        input->lane[candidate->lane].input_pri_out & 1u);
    return true;
}

void t425_core_reset(t425_core_t *core)
{
    if (!core)
        return;

    t425_cpu_reset(&core->cpu);
    t425_microsequencer_reset(&core->microsequencer, 0u);
    core->microentry_signal = 0u;

    /*
     * FETCH is the reset state.  The next core step performs
     * instruction fetch and establishes the microcode entry.
     */
    core->step_instruction = 0u;
    core->next_step_instruction = 0u;

    t425_datapath_reset(&core->datapath);
    t425_memory_reset(&core->memory);
    t425_timer_reset(&core->timer);
    t425_scheduler_reset(&core->scheduler);
    core->t425_channel_select = 0u;
    core->t425_channel_stack = 0u;
    t425_channel_dma_bank_init(&core->t425_channel_dma);
    t425_channel_admission_21_init(&core->channel_admission);
    (void)t425_channel_dma_memory_04_init(
        &core->channel_dma_memory, &core->memory, &core->t425_channel_dma);
    (void)t425_channel_dma_shift_05_init(
        &core->channel_dma_shift, &core->t425_channel_dma,
        &core->channel_dma_memory);
    core->t425_data_mem_addr = 0u;
    core->experimental_data_mem_addr_from_wdesc = 0u;
    core->t425_data_reg = 0u;
    core->previous_read_data = 0u;
    core->data_in = 0u;
    core->byte_write_strobe_latch = 0u;
    core->byte_write_pending = 0u;
    core->byte_write_pending_mask = 0u;
    core->byte_write_pending_clear_latch = 0u;
    core->byte_write_pending_address = 0u;
    core->byte_write_pending_data = 0u;
    core->byte_write_pending_iptr = 0u;
    core->byte_write_pending_mpc = 0u;
    core->data_in_reg = 0u;
    core->memory_read_pending = 0u;
    core->memory_read_pending_areg = 0u;
    core->memory_read_pending_address = 0u;
    core->two_reads_latch = 0u;
    core->clear_bwse_latch_after_write = 0u;
    core->t425_channel_data_reg = 0u;
    core->clock_reg0 = 0u;
    core->clock_reg1 = 0u;
    core->t425_channel_ready_not_run = 0u;
    core->t425_channel_run_req = 0u;
    memset(&core->channel_vbc_state, 0, sizeof(core->channel_vbc_state));
    core->t425_channel_status = 0u;
    core->t425_alt_enabled = 0u;
    core->phy = 0;
    memset(&core->channel_dma_rx_adapter_10, 0,
           sizeof(core->channel_dma_rx_adapter_10));
    core->channel_dma_rx_adapter_10_initialized = 0u;
    core->channel_compat_profile_08_context = NULL;
    core->channel_compat_profile_08_microstep = NULL;
    core->channel_compat_profile_08_reduce = NULL;
    core->channel_compat_profile_08_clear = NULL;
    core->channel_compat_profile_08_service = NULL;
    core->reset_active = 0u;
    core->bootfromrom_sampled = 0u;
    core->bootfromrom_value = 0u;
    core->boot_state = T425_BOOT_RESET;
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
    for (uint32_t i = 0u; i < T425_CORE_PHY_MAX_ENDPOINTS; ++i) {
        core->external_in_active[i] = 0u;
        core->external_in_seen[i] = 0u;
        core->external_in_remaining[i] = 0u;
    }

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

    core->cycles = 0;
    core->instructions = 0;
}

bool t425_core_attach_phy(
    t425_core_t *core,
    t425_core_phy_t *phy)
{
    if (core == 0)
        return false;

    if (core->channel_dma_rx_adapter_10_initialized &&
        core->channel_dma_rx_adapter_10.core0_phy != phy) {
        for (uint32_t i = 0u;
             i < T425_CHANNEL_DMA_RX_ADAPTER_10_ENDPOINTS; ++i) {
            const t425_channel_dma_rx_adapter_10_endpoint_t *route =
                &core->channel_dma_rx_adapter_10.endpoint[i];
            if (route->active || route->held_valid || route->drain_pending ||
                route->session_bound)
                return false;
        }
        core->channel_dma_rx_adapter_10_initialized = 0u;
    }

    if (phy != 0 && phy->attached) {
        if (!core->channel_dma_rx_adapter_10_initialized) {
            if (!t425_channel_dma_rx_adapter_10_init(
                    &core->channel_dma_rx_adapter_10, phy,
                    &core->channel_dma_shift))
                return false;
            core->channel_dma_rx_adapter_10_initialized = 1u;
        }
    } else if (phy == 0) {
        if (core->channel_dma_rx_adapter_10_initialized) {
            for (uint32_t i = 0u;
                 i < T425_CHANNEL_DMA_RX_ADAPTER_10_ENDPOINTS; ++i) {
                const t425_channel_dma_rx_adapter_10_endpoint_t *route =
                    &core->channel_dma_rx_adapter_10.endpoint[i];
                if (route->active || route->held_valid ||
                    route->drain_pending || route->session_bound)
                    return false;
            }
        }
        memset(&core->channel_dma_rx_adapter_10, 0,
               sizeof(core->channel_dma_rx_adapter_10));
        core->channel_dma_rx_adapter_10_initialized = 0u;
    }

    core->phy = phy;
    return true;
}

int t425_core_link_output(
    t425_core_t *core,
    uint32_t link_id)
{
    uint32_t frame;

    if (core == 0 || link_id >= T425_CORE_PHY_MAX_ENDPOINTS ||
        core->phy == 0)
        return -1;

    frame = core->t425_channel_data_reg;
    if (frame > 0xFFu)
        return -1;

    return t425_core_phy_tx(core->phy, link_id, (uint16_t)frame);
}

bool t425_core_link_input_available(
    t425_core_t *core,
    uint32_t link_id)
{
    if (core == 0 || link_id >= T425_CORE_PHY_MAX_ENDPOINTS ||
        core->phy == 0)
        return false;

    return t425_core_phy_rx_available(core->phy, link_id);
}

bool t425_core_link_rx_admission(
    const t425_core_t *core,
    uint32_t link_id)
{
    uint32_t address;
    uint32_t control;

    if (core == 0 || link_id >= T425_CORE_PHY_MAX_ENDPOINTS)
        return false;

    address = 0x80000010u + (link_id * 4u);
    if (t425_memory_read_word(&core->memory, address, &control) != 0)
        return false;

    if (core->external_in_seen[link_id] != 0u)
        return core->external_in_active[link_id] != 0u &&
               core->external_in_remaining[link_id] != 0u;

    return control != T425_CHANNEL_EMPTY;
}

int t425_core_external_in_begin(
    t425_core_t *core, uint32_t link_id, uint32_t byte_count)
{
    if (core == 0 || link_id >= T425_CORE_PHY_MAX_ENDPOINTS ||
        byte_count == 0u)
        return -1;
    core->external_in_active[link_id] = 1u;
    core->external_in_seen[link_id] = 1u;
    core->external_in_remaining[link_id] = byte_count;
    return 0;
}

uint32_t t425_core_external_in_remaining(
    const t425_core_t *core, uint32_t link_id)
{
    if (core == 0 || link_id >= T425_CORE_PHY_MAX_ENDPOINTS)
        return 0u;
    return core->external_in_remaining[link_id];
}

int t425_core_link_input_consume(
    t425_core_t *core,
    uint32_t link_id,
    uint8_t *byte)
{
    uint32_t frame;
    int result;

    if (core == 0 || byte == 0 ||
        link_id >= T425_CORE_PHY_MAX_ENDPOINTS ||
        core->phy == 0)
        return -1;

    frame = 0u;
    result = t425_core_phy_rx(core->phy, link_id, &frame);
    if (result != 0 || frame > 0xFFu)
        return (result != 0) ? result : -1;

    core->t425_channel_data_reg = frame;
    *byte = (uint8_t)frame;
    if (core->boot_state != T425_BOOT_RUNNING) {
        core->rx_credit_rearm[link_id] = 1u;
    } else if (core->external_in_active[link_id] != 0u) {
        if (core->external_in_remaining[link_id] > 0u)
            --core->external_in_remaining[link_id];
        if (core->external_in_remaining[link_id] == 0u)
            core->external_in_active[link_id] = 0u;
        else
            core->rx_credit_rearm[link_id] = 1u;
    }
    return 0;
}





void t425_core_microcycle(t425_core_t *core)
{
    t425_condition_signals_t signals;
    uint8_t interface_control;
    uint32_t data_reg_source;
    uint32_t data_in_source;
    uint32_t previous_read_source;
    uint32_t previous_read_after_byte_output;
    uint32_t channel_push_sequence_before;
    uint16_t channel_control_current;
    uint8_t compat_input_req = 0u;
    f250_ifnext_candidate_t f250_candidate = {0};
    uint8_t f250_dispatch_ready = 0u;
    uint8_t f250_dispatch_committed = 0u;
    uint16_t f250_dispatch_vector = 0u;
    t425_ifnext_requests_34_t f250_requests = {0};
    t425_ifnext_result_34_t f250_ifnext_result = T425_IFNEXT_34_NONE;
    uint8_t timer_clock_load = 0u;
    uint8_t byte_latch_at_entry;
    uint8_t byte_strobe_enable_current;
    uint8_t data_reg_from_byte_shifter;
    uint8_t prevread_byte_write_enabled;
    uint32_t bytewrite_a_pre;
    uint32_t bytewrite_b_pre;
    uint32_t bytewrite_d_pre;






    if (!core)
        return;
    {}

    /*
     * Capture the interface request belonging to the MIR that is
     * about to execute.  The request belongs to this microcycle;
     * therefore it must be sampled before the microsequencer loads
     * the next MIR.
     */
    interface_control =
        (uint8_t)(t425_mir_interface(
            &core->microsequencer.mir) & 0x7Fu);
    timer_clock_load = (uint8_t)(
        (t425_mir_zbus(&core->microsequencer.mir) & 0x00010u) != 0u);

    {}





    byte_latch_at_entry = core->byte_write_strobe_latch;
    byte_strobe_enable_current = (uint8_t)(
        (t425_mir_byte(&core->microsequencer.mir) >> 1) & 1u);
    bytewrite_a_pre = core->cpu.A;
    bytewrite_b_pre = core->cpu.B;
    bytewrite_d_pre = core->cpu.Dreg;

    /*
     * The EMUL64 MemoryInterface consumes the prior microcycle's bus-arbiter
     * latch before this microword performs its read and datapath operations.
     */
    {}
    if (core->byte_write_pending != 0u) {
        if (0) {}
        uint8_t pending_mask = core->byte_write_pending_mask;
        int write_rc = pending_mask == 0u ? 0 :
            t425_memory_write_masked_word(
                &core->memory, core->byte_write_pending_address,
                core->byte_write_pending_data, pending_mask);
        if (write_rc != 0) {
            core->cpu.error = 1u;
        } else {
            if (pending_mask != 0u &&
                core->byte_write_pending_address == 0x800006f4u) {
                uint32_t f95_word_post = 0u;
                int f95_post_rc = t425_memory_read_word(
                    &core->memory, core->byte_write_pending_address,
                    &f95_word_post);
                {}
            }
            if (pending_mask == 0x0Fu &&
                core->byte_write_pending_address == core->t425_channel_select &&
                core->byte_write_pending_data == core->cpu.WdescReg) {
                {
                    uint32_t f77addr = (core->byte_write_pending_address);
                    uint32_t f77data = (core->byte_write_pending_data);
                    t425_channel_admission_entry_21_t *f77e = &core->channel_admission.channel[4];
                    unsigned f77state = (unsigned)f77e->state;
                    if (f77addr == 0x80000000u) {} bool f77accepted = t425_channel_admission_21_observe_channel_write(&core->channel_admission, &core->t425_channel_dma, f77addr, f77data); if (f77addr == 0x80000000u) {}
                    if (f77addr == 0x80000000u) {}
                }
            }
            if (core->byte_write_pending_clear_latch != 0u) {
                {}
                /* Consume legacy pending-clear metadata without clearing the latch. */
                core->clear_bwse_latch_after_write = 0u;
            }
        }
        core->byte_write_pending = 0u;
        core->byte_write_pending_mask = 0u;
        core->byte_write_pending_clear_latch = 0u;
    }

    /* Complete the previous microword read before this microword's XBUS. */
    if (core->memory_read_pending != 0u) {
        uint32_t value = 0u;
        uint32_t address = core->memory_read_pending_address;
        uint8_t load_areg = core->memory_read_pending_areg;

        core->memory_read_pending = 0u;
        core->memory_read_pending_areg = 0u;
        if (t425_memory_read_word(&core->memory, address, &value) == 0) {
            core->previous_read_data = core->data_in_reg;
            core->data_in_reg = value;
            core->data_in = value;
            if ((interface_control & 0x10u) != 0u)
                core->previous_read_data = value;
            if (load_areg != 0u)
                core->cpu.A = value;
        } else {
            core->cpu.error = 1u;
        }
    }

    /*
     * DataRegfromAreg is sampled from Areg at the beginning of
     * the microword.  This is essential for microwords such as
     * STL, where DataRegfromAreg and STACK=14 coexist.
     *
     * STACK=14 subsequently performs:
     *
     *     A <- old B
     *     B <- old C
     *
     * Therefore DataRegfromAreg must retain old A, not the
     * post-STACK value.
     */
    data_in_source = core->data_in_reg;
    previous_read_source = core->previous_read_data;
    data_reg_source = core->cpu.ASlave;
    data_reg_from_byte_shifter = (uint8_t)(
        t425_mir_byte(&core->microsequencer.mir) & 0x01u);
    channel_push_sequence_before = core->t425_channel_dma.push_event_sequence;

    if (t425_channel_admission_21_is_compat_input(&core->channel_admission)) {
        if (core->channel_compat_profile_08_microstep != NULL)
            core->channel_compat_profile_08_microstep(
                core->channel_compat_profile_08_context, core);
        if (core->channel_compat_profile_08_reduce != NULL)
            compat_input_req = core->channel_compat_profile_08_reduce(
                core->channel_compat_profile_08_context);
    }

    signals = (t425_condition_signals_t){0};
    /*
     * MV-01/MV-02: VBC emulator parity for the MOVE start predicate.
     * Sample architectural B/C before this microword commits datapath
     * destinations; Cond1 selector 8 consumes its complement later in
     * this same microcycle.
     */
    core->two_reads_latch = (uint8_t)(
        ((core->cpu.C & 3u) > (core->cpu.B & 3u)) ? 1u : 0u);
    signals.not_two_reads_to_start =
        (uint8_t)(core->two_reads_latch ^ 1u);
    signals.proc_priority = (uint8_t)(core->cpu.WdescReg & 1u);
    signals.ch_or_timer_req = (uint8_t)(
        (core->t425_channel_run_req | compat_input_req |
         channel_vbc_output_run_request_reduce(&core->channel_vbc_state)) & 1u);
    signals.ch_not_timer_req = (uint8_t)(
        (core->t425_channel_run_req | compat_input_req |
         channel_vbc_output_run_request_reduce(&core->channel_vbc_state)) & 1u);
    signals.channel_ready_not_run =
        (uint8_t)(core->t425_channel_ready_not_run & 1u);

    /*
     * F20-10 — ByteWriteStrobeEnable.
     *
     * VBC:
     *
     *     ByteWriteStrobeEnable = #B10
     *     DataRegfromByteShifter = #B01
     *
     * The ByteWriteStrobeEnable request SETs the persistent
     * ByteWriteStrobeEnableLatch before ByteUpShifter evaluates
     * its input state.
     *
     * MIR[108:107] = 10 -> SET latch.
     */
    if ((t425_mir_byte(&core->microsequencer.mir) & 0x02u) != 0u) {
        if (core->byte_write_strobe_latch == 0u)
            if (0) {}
        core->byte_write_strobe_latch = 1u;
    }

    /*
     * F20-10-05E — ByteUpShifter input is the physical DataInReg.
     *
     * DataInReg is the current memory-read result.
     * PreviousReadReg remains a separate temporal register and is
     * supplied independently below.
     *
     * DataInReg is the authoritative functional DataIn source. The
     * public data_in field is kept as a synchronized compatibility
     * mirror at the interface commit points below.
     */
    core->datapath.channel_stack_bank = &core->t425_channel_dma;
    t425_datapath_execute(
        &core->datapath,
        &core->cpu,
        &core->t425_channel_select,
        &core->t425_channel_stack,
        &core->t425_data_mem_addr,
        &core->experimental_data_mem_addr_from_wdesc,
        core->t425_channel_data_reg,
        &core->scheduler.fptr[0],
        &core->scheduler.fptr[1],
        &core->scheduler.bptr[0],
        &core->scheduler.bptr[1],
        &core->clock_reg0,
        &core->clock_reg1,
        &core->timer.next_high,
        &core->timer.next_low,
        &core->microsequencer.mir,
        data_in_source,
        core->data_in_reg,
        previous_read_source,
        core->byte_write_strobe_latch,
        core->microsequencer.mpc,
        &signals);

    /* F98 passive snapshot: datapath has consumed the post-SET latch. */
    {}

    /* Observe the actual accepted ChannelStack push after datapath execution. */
    if (core->t425_channel_dma.push_event_sequence !=
        channel_push_sequence_before) {
        {
            t425_channel_admission_entry_21_t *f77e =
                &core->channel_admission.channel[4];
            unsigned f77state = (unsigned)f77e->state;
            bool f77accepted;
            uint32_t f79arch = core->t425_channel_dma.last_push_architectural_channel;
            uint32_t f79value = core->t425_channel_dma.last_push_value;
            uint16_t f79mpc = core->microsequencer.mpc;
            if (f79arch == 0x80000000u && (f79mpc == 0x282u || f79mpc == 0x289u))
                {}
            f77accepted = t425_channel_admission_21_observe_push(
                &core->channel_admission, &core->t425_channel_dma, f79arch, f79value);
            if (f79arch == 0x80000000u && (f79mpc == 0x282u || f79mpc == 0x289u))
                {}
            if (core->t425_channel_dma.last_push_architectural_channel == 0x80000000u)
                {}
        }
    }

    /*
     * F19-02 — physical Channel Control Requests.
     *
     * RESETCH microstate 1B3 contains CH=0x0C2:
     *
     *   bit 7 : ResetChannel
     *   bit 6 : ResetChannelReadyNotRunlatch
     *   bit 1 : ResetChannelRunReqlatch
     *
     * These are architectural effects of the current microword.
     * They must be sampled in the same microcycle as the MIR.
     *
     * Do not synthesize rendezvous/FIFO state here.
     */
    channel_control_current =
        (uint16_t)(t425_mir_channel(&core->microsequencer.mir) & 0x07FFu);
    {
        uint16_t channel_control = channel_control_current;

        if ((channel_control & 0x001u) != 0u) {
            core->t425_data_mem_addr = core->t425_channel_select;
            core->experimental_data_mem_addr_from_wdesc = 0u;
        }

        if ((channel_control & 0x080u) != 0u) {
            if (t425_channel_reset(
                    &core->memory,
                    core->t425_channel_select) != 0)
                core->cpu.error = 1u;
        }

        if ((channel_control & 0x040u) != 0u)
            core->t425_channel_ready_not_run = 0u;

        if ((channel_control & 0x020u) != 0u)
            core->t425_alt_enabled = 1u;

        if ((channel_control & 0x010u) != 0u)
            core->t425_alt_enabled = 0u;

        if ((channel_control & 0x008u) != 0u) {
            const t425_channel_route_52_t *selected_route =
                channel_route_52_find_28(core->t425_channel_select);
            core->t425_channel_status = 0u;
            if (selected_route != NULL &&
                selected_route->direction == T425_CHANNEL_DMA_TX)
                (void)channel_vbc_output_gate_clear(
                    &core->channel_vbc_state, selected_route->dma_slot);

            /* Disable only the selected INPUT lane. Signals for this cycle
             * were snapshotted earlier, so the next reduction observes it. */
            if (t425_channel_admission_21_is_compat_input(
                    &core->channel_admission) &&
                core->channel_compat_profile_08_context != NULL) {
                t425_channel_compat_rx_adapter_07_t *adapter =
                    (t425_channel_compat_rx_adapter_07_t *)
                        core->channel_compat_profile_08_context;
                uint8_t input_lane, stack_lane, shared_index;

                if (adapter->input != NULL &&
                    t425_channel_compat_input_decode_address(
                        core->t425_channel_select, &input_lane,
                        &stack_lane, &shared_index) &&
                    t425_channel_compat_input_disable_selected(
                        adapter->input, core->t425_channel_select)) {
                    const t425_channel_compat_input_lane_t *lane =
                        &adapter->input->lane[input_lane];
                    t425_channel_admission_entry_21_t *entry =
                        &core->channel_admission.channel[input_lane];

                    if (lane->active == 1u &&
                        lane->request_id != 0u && lane->generation != 0u &&
                        lane->route.architectural_channel ==
                            core->t425_channel_select &&
                        lane->route.input_lane == input_lane &&
                        lane->route.stack_lane == stack_lane &&
                        lane->route.shared_index == shared_index &&
                        entry->state == T425_CHANNEL_ADMISSION_21_ACTIVE &&
                        entry->architectural_channel ==
                            core->t425_channel_select &&
                        entry->direction == T425_CHANNEL_DMA_RX &&
                        entry->lane == input_lane &&
                        entry->endpoint == lane->route.endpoint_id &&
                        entry->physical_link == lane->route.physical_link &&
                        entry->request_id == lane->request_id &&
                        entry->operation_generation == lane->generation)
                        entry->enabled = 0u;
                }
            }
        }

        if ((channel_control & 0x004u) != 0u) {
            const t425_channel_route_52_t *selected_route =
                channel_route_52_find_28(core->t425_channel_select);
            uint32_t endpoint = T425_CORE_PHY_MAX_ENDPOINTS;
            uint32_t selected = core->t425_channel_select;
            uint32_t byte_count = core->cpu.B;

            core->t425_channel_status = 1u;
            /* Output VBC gate is separate from DMA admission. */
            if (selected_route != NULL &&
                selected_route->direction == T425_CHANNEL_DMA_TX)
                (void)channel_vbc_output_gate_set(
                    &core->channel_vbc_state, selected_route->dma_slot);
            {
                t425_channel_admission_entry_21_t *f77e =
                    &core->channel_admission.channel[4];
                unsigned f77state = (unsigned)f77e->state;
                if (selected == 0x80000000u) {}
                bool f77accepted = t425_channel_admission_21_observe_enable(
                    &core->channel_admission, selected);
                if (selected == 0x80000000u) {}
                if (selected == 0x80000000u)
                    {}
            }

            if (selected == 0x80000010u) endpoint = 0u;
            else if (selected == 0x80000014u) endpoint = 1u;
            else if (selected == 0x80000018u) endpoint = 2u;
            else if (selected == 0x8000001Cu) endpoint = 3u;

            if (endpoint < T425_CORE_PHY_MAX_ENDPOINTS &&
                byte_count != 0u &&
                core->external_in_active[endpoint] == 0u &&
                core->external_in_remaining[endpoint] == 0u)
                (void)t425_core_external_in_begin(
                    core, endpoint, byte_count);
        }

        if ((channel_control & 0x002u) != 0u) {
            core->t425_channel_run_req = 0u;
            /* Per-output reset needs selected OutputChannelStatus == 0xA.
             * This CORE has no equivalent output-status state; leave requests.
             */
        }
    }

    /*
     * DataRegfromAreg.
     *
     * The source is the A value sampled at the beginning of this
     * microword, before any STACK destination is applied.
     */
    if ((interface_control & 0x01u) != 0u)
        core->t425_data_reg = data_reg_source;

    /*
     * DataRegfromZbus is an independent VBC interface-control
     * request.  The datapath has already evaluated the current
     * microword, so datapath->zbus is the physical Z-bus value
     * produced by that microword.
     *
     * CALL state 269 is the decisive case:
     *
     *     X = IPTR
     *     ALU = X
     *     Z = IPTR
     *     INTERFACE = MemWrite | DataRegfromZbus = 0x0A
     *
     * Therefore the value written by MemWrite must be the
     * Z-bus value from this same microcycle.
     */
    if ((interface_control & 0x02u) != 0u)
        core->t425_data_reg = core->datapath.zbus;

    /*
     * F20-09 — DataRegfromByteShifter.
     *
     * MIR[108:107] = 01 selects the physical ByteUpShifter
     * output as the source of DataReg.
     *
     * The ByteUpShifter result was already produced by the
     * datapath during this same microword.
     */
    if ((core->datapath.byte_control & 0x01u) != 0u)
        core->t425_data_reg = core->datapath.byte_upshifter_output;
    {}

    /*
     * MemRead is sampled after the datapath has produced
     * DataMemAddrFromZbus for this microword.
     *
     * The result becomes previous_read_data and is therefore
     * available to DataInfromPrevReadReg in a later temporal
     * point.
     */
    if ((interface_control & 0x04u) != 0u) {
        core->memory_read_pending = 1u;
        core->memory_read_pending_address =
            core->t425_data_mem_addr & ~3u;
        core->memory_read_pending_areg =
            (interface_control & 0x20u) != 0u ? 1u : 0u;
    }

    /*
     * F87 experimental EMUL64 ordering: DataOutfromByteUpShifterOutput
     * runs during datapath evaluation, before determineZbusDestination.
     * Under the REA gates, it updates PreviousReadReg from DataInReg.
     * DataInfromPrevReadReg then consumes that updated value. Keep the
     * PRE snapshots explicit so C assignment order does not define this
     * experimental behavior accidentally.
     */
    prevread_byte_write_enabled = (uint8_t)
        t425_core_f87_prevread_write_enabled(
            data_reg_from_byte_shifter,
            byte_strobe_enable_current,
            core->byte_write_strobe_latch,
            (uint8_t)((interface_control & 0x08u) != 0u));
    previous_read_after_byte_output =
        t425_core_f87_prevread_next(
            previous_read_source,
            data_in_source,
            data_reg_from_byte_shifter,
            byte_strobe_enable_current,
            core->byte_write_strobe_latch,
            (uint8_t)((interface_control & 0x08u) != 0u));
    if (prevread_byte_write_enabled != 0u)
        core->previous_read_data = previous_read_after_byte_output;

    /*
     * Commit DataRegfromAreg and DataInfromPrevReadReg after datapath
     * consumption. MIR[77] selects the source captured in
     * PreviousReadReg; IF=0x10 with MemRead consumes the newly returned
     * DataInReg value. These pre-existing paths remain in their order.
     */
    if ((interface_control & 0x01u) != 0u) {
        core->previous_read_data = core->microsequencer.mir.bit[77] ?
            data_reg_source : data_in_source;
        core->data_in_reg = data_reg_source;
        core->data_in = data_reg_source;
    }
    if ((interface_control & 0x10u) != 0u) {
        if ((interface_control & 0x04u) != 0u)
            core->previous_read_data = core->data_in_reg;
        core->data_in_reg = core->previous_read_data;
        core->data_in = core->previous_read_data;
    }
    if ((interface_control & 0x20u) != 0u &&
        (interface_control & 0x04u) == 0u)
        if ((interface_control & 0x10u) != 0u &&
            data_reg_from_byte_shifter != 0u)
            core->cpu.A = core->datapath.byte_upshifter_output;
        else
            core->cpu.A = core->data_in_reg;

    {}

    /*
     * F20-10-05D — physical ClearBWSELatchAfterWrite.
     *
     * Emulator condition:
     *   DataInReg[3] && MPC != 0x283 && TwoReadsLatch
     *
     * DataInReg is the current memory-read register.
     * data_in is deliberately NOT used here because it
     * represents DataInfromPrevReadReg.
     */
    if (((core->data_in_reg & 0x08u) != 0u) &&
        (core->microsequencer.mpc != 0x283u) &&
        (core->two_reads_latch != 0u))
    {
        if (core->clear_bwse_latch_after_write == 0u)
            if (0) {}
        core->clear_bwse_latch_after_write = 1u;
    }

    /*
     * MemWrite consumes the DataReg selected by the current
     * interface-control request.
     *
     * DataMemAddrFromZbus was produced by the datapath during
     * this same microword.
     */
    if ((interface_control & 0x08u) != 0u) {
        uint32_t raw_address = core->t425_data_mem_addr;
        uint8_t mir_byte_field = t425_mir_byte(&core->microsequencer.mir);
        uint8_t datareg_from_byte_shifter = (uint8_t)(mir_byte_field & 1u);
        /* Byte strobe state qualifies only ByteUpShifter payloads. */
        uint8_t byte_mode = (uint8_t)(
            datareg_from_byte_shifter != 0u &&
            (byte_strobe_enable_current != 0u ||
             byte_latch_at_entry != 0u));
        /*
         * Keep the VBC byte address intact. The memory backend uses a word
         * base plus lane strobes; normalize only non-byte writes here.
         */
        uint32_t committed_address =
            core->experimental_data_mem_addr_from_wdesc != 0u
                ? (raw_address & ~1u)
                : (byte_mode != 0u ? raw_address : (raw_address & ~3u));
        if (0) {}
        if (byte_mode != 0u) {
            if (core->byte_write_pending != 0u) {
                core->cpu.error = 1u;
            } else {
                core->byte_write_pending_address = raw_address & ~3u;
                core->byte_write_pending_data = core->t425_data_reg;
                core->byte_write_pending_mask = t425_byte_write_lane_mask(
                    bytewrite_a_pre, bytewrite_b_pre, bytewrite_d_pre,
                    byte_strobe_enable_current, byte_latch_at_entry);
                core->byte_write_pending_clear_latch =
                    core->clear_bwse_latch_after_write;
                core->byte_write_pending_iptr = core->cpu.IptrReg;
                core->byte_write_pending_mpc = core->microsequencer.mpc;
                core->byte_write_pending = 1u;
                if (0) {}
            }
        } else {
            uint32_t pre_word = 0u;
            int pre_rc = t425_memory_read_word(
                &core->memory, committed_address, &pre_word);
            if (0) {}
            if (t425_memory_write_word(
                    &core->memory, committed_address,
                    core->t425_data_reg) != 0) {
            if (0) {}
                core->cpu.error = 1u;
            } else {
            {}
            if (committed_address == core->t425_channel_select &&
                core->t425_data_reg == core->cpu.WdescReg) {
                {
                    uint32_t f77addr = (committed_address);
                    uint32_t f77data = (core->t425_data_reg);
                    t425_channel_admission_entry_21_t *f77e = &core->channel_admission.channel[4];
                    unsigned f77state = (unsigned)f77e->state;
                    if (f77addr == 0x80000000u) {} bool f77accepted = t425_channel_admission_21_observe_channel_write(&core->channel_admission, &core->t425_channel_dma, f77addr, f77data); if (f77addr == 0x80000000u) {}
                    if (f77addr == 0x80000000u) {}
                }
            }
            if (core->clear_bwse_latch_after_write != 0u) {
                /* F95: commit retires only the legacy metadata, not the latch. */
                {}
                core->clear_bwse_latch_after_write = 0u;
            }
            }
        }
    }

    /*
     * F98 defers Next-clear until this microword has evaluated datapath and
     * captured DataReg/MemWrite pending state. This preserves the current
     * cycle ByteUp result and lane payload; only persistent latch state for
     * the following cycle is cleared. Predicate remains Next-only as in F95.
     */
    if ((interface_control & 0x40u) != 0u) {
        if (core->byte_write_strobe_latch != 0u)
            if (getenv("F190_VERBOSE")) {}
        core->byte_write_strobe_latch = 0u;
        core->clear_bwse_latch_after_write = 0u;
        if (getenv("F190_VERBOSE")) {}
    }

    /* IF Next uses the F250 owner arbitration for compat INPUT requests.
     * Other eligible channel/timer requests retain the established F245
     * signal-based vector selection. */
    if ((interface_control & 0x40u) != 0u) {
        uint8_t compat_input_boundary = (uint8_t)(
            compat_input_req != 0u &&
            t425_channel_admission_21_is_compat_input(
                &core->channel_admission) &&
            core->channel_compat_profile_08_context != NULL);
        if (compat_input_boundary != 0u) {
            t425_channel_compat_rx_adapter_07_t *adapter =
                (t425_channel_compat_rx_adapter_07_t *)
                    core->channel_compat_profile_08_context;
            if (adapter->input != NULL &&
                f250_ifnext_unique_input(adapter->input, &f250_candidate)) {
                if (f250_candidate.priority_high != 0u)
                    f250_requests.channel_high = 1u;
                else
                    f250_requests.channel_low = 1u;
                f250_ifnext_result = t425_ifnext_select_34(
                    &f250_requests, signals.proc_priority, 1u,
                    &f250_dispatch_vector);
                if (f250_ifnext_result == T425_IFNEXT_34_SELECTED)
                    f250_dispatch_ready = 1u;
            }
        } else {
            uint8_t eligible = signals.proc_priority != 0u
                ? signals.ch_or_timer_req
                : signals.high_not_low_req;
            if (eligible != 0u) {
                f250_dispatch_vector = (uint16_t)(0x1F8u |
                    ((signals.ch_not_timer_req & 1u) << 1) |
                    (signals.high_not_low_req & 1u));
                f250_dispatch_ready = 2u;
            }
        }
    }

    t425_microsequencer_step(
        &core->microsequencer,
        &signals);

    if (f250_dispatch_ready == 1u) {
        t425_channel_compat_rx_adapter_07_t *adapter =
            (t425_channel_compat_rx_adapter_07_t *)
                core->channel_compat_profile_08_context;
        t425_channel_compat_input_key_t bound = {0};
        t425_channel_ifnext_owner_result_t owner_status =
            adapter != NULL ? t425_channel_compat_rx_adapter_07_bind_ifnext_owner(
                adapter, true, &bound) : T425_CHANNEL_IFNEXT_OWNER_NONE;
        if (owner_status == T425_CHANNEL_IFNEXT_OWNER_SELECTED &&
            f250_same_key(&bound, &f250_candidate.key)) {
            core->t425_channel_select =
                f250_candidate.key.route.architectural_channel;
            core->microsequencer.next_mpc = f250_dispatch_vector;
            core->microsequencer.mpc = f250_dispatch_vector;
            t425_microsequencer_load_mir(&core->microsequencer);
            f250_dispatch_committed = 1u;
        }
    } else if (f250_dispatch_ready == 2u) {
        core->microsequencer.next_mpc = f250_dispatch_vector;
        core->microsequencer.mpc = f250_dispatch_vector;
        t425_microsequencer_load_mir(&core->microsequencer);
        f250_dispatch_committed = 1u;
    }

    if (t425_channel_admission_21_is_compat_input(&core->channel_admission) &&
        core->channel_compat_profile_08_clear != NULL)
        (void)core->channel_compat_profile_08_clear(
            core->channel_compat_profile_08_context,
            core->t425_channel_select, channel_control_current);

    /*
     * Next is the VBC Interface Control Request:
     *
     *     Next = #B1000000
     *
     * It requests return to the instruction-level FETCH stage.
     * It is not interpreted here as "MPC + 1"; the microsequencer
     * retains its already-established physical next-state behavior.
     */
    /* F250: retain normal FETCH unless a unique, eligible live owner was
     * revalidated and dispatched to its existing CROM vector. */
    core->next_step_instruction =
        ((interface_control & 0x40u) != 0u)
            ? (f250_dispatch_committed != 0u ? 1u : 0u) : 1u;
    /* F172: advance one simulated time unit per completed CORE microcycle.
     * A ClockReg destination loads the visible value while preserving the
     * periodic divider phase. No host wall clock is used. */
    if (timer_clock_load != 0u) {
        core->timer.high = core->clock_reg0;
        core->timer.low = core->clock_reg1;
    }
    t425_timer_advance(&core->timer, 1u);
    core->clock_reg0 = core->timer.high;
    core->clock_reg1 = core->timer.low;

    {}
}

static const uint32_t channel_dma_handoff_addresses_28[
    T425_CHANNEL_ADMISSION_21_CHANNELS] = {
    0x80000010u, 0x80000014u, 0x80000018u, 0x8000001Cu,
    0x80000000u, 0x80000004u, 0x80000008u, 0x8000000Cu
};

static bool channel_dma_handoff_resolve_28(
    uint32_t address, uint8_t *index, uint8_t *direction, uint8_t *lane)
{
    for (uint8_t i = 0u; i < T425_CHANNEL_ADMISSION_21_CHANNELS; ++i) {
        if (channel_dma_handoff_addresses_28[i] == address) {
            *index = i;
            *direction = i < 4u ? T425_CHANNEL_DMA_RX : T425_CHANNEL_DMA_TX;
            *lane = (uint8_t)(i & 3u);
            return true;
        }
    }
    return false;
}

static const t425_channel_dma_lane_t *channel_dma_handoff_const_lane_28(
    const t425_channel_dma_bank_t *bank, uint8_t direction, uint8_t lane)
{
    if (bank == NULL || lane >= 4u)
        return NULL;
    return direction == T425_CHANNEL_DMA_RX ? &bank->rx[lane] :
           direction == T425_CHANNEL_DMA_TX ? &bank->tx[lane] : NULL;
}

static bool channel_dma_handoff_core_ready_28(const t425_core_t *core)
{
    return core != NULL &&
           core->channel_dma_shift.bank == &core->t425_channel_dma &&
           core->channel_dma_memory.bank == &core->t425_channel_dma &&
           core->channel_dma_memory.memory == &core->memory &&
           core->channel_dma_shift.memory == &core->channel_dma_memory;
}

static bool channel_dma_handoff_same_request_28(
    const t425_channel_admission_request_21_t *a,
    const t425_channel_admission_request_21_t *b)
{
    return a->request_id == b->request_id &&
           a->operation_generation == b->operation_generation &&
           a->architectural_channel == b->architectural_channel &&
           a->buffer_address == b->buffer_address &&
           a->initial_count_reg == b->initial_count_reg &&
           a->length == b->length && a->channel_word == b->channel_word &&
           a->direction == b->direction && a->lane == b->lane &&
           a->endpoint == b->endpoint && a->physical_link == b->physical_link;
}

static const t425_channel_route_52_t *channel_route_52_find_28(
    uint32_t address)
{
    for (size_t i = 0u; i < T425_CHANNEL_ROUTE_52_COUNT; ++i)
        if (t425_channel_routes_52[i].architectural_channel == address)
            return &t425_channel_routes_52[i];
    return NULL;
}

static t425_channel_dma_handoff_28_status_t
channel_dma_handoff_prepare_compat_rx_28(
    const t425_core_t *core, uint32_t address, uint8_t index, uint8_t lane_index,
    const t425_channel_admission_request_21_t *request,
    t425_channel_dma_handoff_28_plan_t *plan)
{
    t425_channel_compat_rx_adapter_07_t *adapter;
    const t425_channel_route_52_t *route52;
    t425_channel_admission_21_t admission_candidate;
    t425_channel_compat_input_route_t route;
    t425_channel_compat_input_key_t key;
    t425_channel_compat_rx_07_endpoint_t *endpoint;
    uint8_t lane;

    if (!t425_channel_admission_21_is_compat_input(&core->channel_admission) ||
        request->direction != T425_CHANNEL_DMA_RX ||
        core->channel_compat_profile_08_context == NULL ||
        core->channel_compat_profile_08_service == NULL ||
        core->channel_compat_profile_08_microstep == NULL ||
        core->channel_compat_profile_08_reduce == NULL ||
        core->channel_compat_profile_08_clear == NULL)
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;
    adapter = (t425_channel_compat_rx_adapter_07_t *)
        core->channel_compat_profile_08_context;
    route52 = channel_route_52_find_28(address);
    if (adapter == NULL || adapter->phy != core->phy ||
        adapter->input == NULL || adapter->f04 != &core->channel_dma_memory ||
        core->phy == NULL || !core->phy->attached || route52 == NULL ||
        route52->direction != T425_CHANNEL_DMA_RX ||
        route52->dma_slot != lane_index || route52->endpoint != request->endpoint ||
        route52->physical_link != request->physical_link ||
        request->endpoint >= T425_CHANNEL_COMPAT_RX_ADAPTER_07_ENDPOINTS)
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    /* A profile is exclusive: any live or retained legacy F05 RX state blocks
     * compatible admission. No F05 state is migrated or cleared here. */
    for (uint8_t i = 0u; i < T425_CHANNEL_DMA_SLOT_COUNT; ++i) {
        const t425_channel_dma_05_rx_state_t *rx =
            &core->channel_dma_shift.rx[i];
        const t425_channel_dma_rx_adapter_10_endpoint_t *legacy =
            &core->channel_dma_rx_adapter_10.endpoint[i];
        if (rx->configured || rx->active || legacy->active ||
            legacy->held_valid || legacy->drain_pending || legacy->session_bound)
            return T425_CHANNEL_DMA_HANDOFF_28_BUSY;
    }

    endpoint = &adapter->endpoint[request->endpoint];
    lane = request->lane;
    if (endpoint->bound != 0u || endpoint->held_valid > 1u ||
        (endpoint->held_valid != 0u && endpoint->held_frame > UINT32_C(0xFF)) ||
        lane >= T425_CHANNEL_COMPAT_INPUT_LANES ||
        adapter->input->lane[lane].active != 0u ||
        adapter->input->lane[lane].input_data_valid != 0u ||
        adapter->input->lane[lane].input_ack != 0u ||
        adapter->input->lane[lane].input_complete != 0u ||
        adapter->input->lane[lane].ubus_request_pending != 0u ||
        adapter->input->lane[lane].staging_mask != 0u ||
        adapter->input->lane[lane].push_mailbox.valid != 0u)
        return T425_CHANNEL_DMA_HANDOFF_28_BUSY;
    if (!t425_channel_compat_input_route_from_config(
            address, request->endpoint, request->physical_link,
            adapter->lane_id[lane], &route) ||
        route.stack_lane != request->lane ||
        route.shared_index != (uint8_t)(request->lane + 4u))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    key.route = route;
    key.request_id = request->request_id;
    key.generation = request->operation_generation;
    plan->compat_input_candidate = *adapter->input;
    if (!t425_channel_compat_input_capture_after_pushes_at_offset(
            &plan->compat_input_candidate, &route, request->request_id,
            request->operation_generation, 2u, request->initial_count_reg,
            request->buffer_address & ~UINT32_C(3),
            core->channel_admission.channel[index].initial_dbuff_reg,
            (uint8_t)(request->buffer_address & 3u), true))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    memcpy(&admission_candidate, &core->channel_admission,
           sizeof(admission_candidate));
    if (!t425_channel_admission_21_report_begin_compat_input(
            &admission_candidate, address, request->request_id))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    plan->compat_adapter = adapter;
    plan->compat_endpoint_candidate = *endpoint;
    plan->compat_endpoint_candidate.key = key;
    plan->compat_endpoint_candidate.bound = 1u;
    plan->admission_candidate = admission_candidate.channel[index];
    plan->rx_profile_snapshot = T425_CHANNEL_RX_PROFILE_COMPAT_INPUT_FROZEN;
    memcpy(plan->compat_input_snapshot, adapter->input,
           sizeof(*adapter->input));
    memcpy(plan->compat_endpoint_snapshot, endpoint, sizeof(*endpoint));
    memcpy(plan->legacy_adapter_snapshot, &core->channel_dma_rx_adapter_10,
           sizeof(core->channel_dma_rx_adapter_10));
    return T425_CHANNEL_DMA_HANDOFF_28_PREPARED;
}

t425_channel_dma_handoff_28_status_t t425_core_channel_dma_handoff_28_prepare(
    const t425_core_t *core, uint32_t address,
    t425_channel_dma_handoff_28_plan_t *plan)
{
    t425_channel_admission_request_21_t request = {0};
    t425_channel_admission_21_t admission_candidate;
    t425_channel_dma_bank_t bank_candidate;
    t425_channel_dma_memory_04_t memory_candidate;
    const t425_channel_admission_entry_21_t *entry;
    const t425_channel_dma_lane_t *lane;
    uint8_t index, direction, lane_index;
    uint64_t end;
    t425_channel_dma_05_status_t begin_status;

    if (plan == NULL)
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;
    memset(plan, 0, sizeof(*plan));
    if (!channel_dma_handoff_core_ready_28(core) ||
        !channel_dma_handoff_resolve_28(address, &index, &direction,
                                        &lane_index))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    entry = &core->channel_admission.channel[index];
    lane = channel_dma_handoff_const_lane_28(
        &core->t425_channel_dma, direction, lane_index);
    if (entry->state != T425_CHANNEL_ADMISSION_21_REQUEST_PENDING ||
        lane == NULL ||
        !t425_channel_admission_21_get_request(
            &core->channel_admission, &core->t425_channel_dma, address,
            &request))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    end = (uint64_t)request.buffer_address + request.length;
    if (request.request_id == 0u || request.operation_generation == 0u ||
        request.architectural_channel != address ||
        entry->architectural_channel != address ||
        request.direction != direction || entry->direction != direction ||
        request.lane != lane_index || entry->lane != lane_index ||
        request.endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        request.physical_link >= T425_CORE_PHY_MAX_ENDPOINTS ||
        entry->endpoint != request.endpoint ||
        entry->physical_link != request.physical_link ||
        entry->route_valid == 0u || entry->enabled == 0u ||
        entry->descriptor_valid == 0u ||
        entry->channel_word_published == 0u || entry->push_count != 2u ||
        entry->request_id != request.request_id ||
        entry->operation_generation != request.operation_generation ||
        entry->buffer_address != request.buffer_address ||
        entry->first_push_value != request.buffer_address ||
        entry->length != request.length ||
        entry->initial_count_reg != request.initial_count_reg ||
        entry->initial_dbuff_reg != lane->dbuff_reg ||
        request.buffer_address != lane->ptr_reg ||
        request.initial_count_reg != lane->count_reg ||
        request.initial_count_reg != (uint32_t)(0u - request.length) ||
        request.length == 0u ||
        end > (uint64_t)UINT32_MAX - 3u ||
        (request.length != 0u &&
         (request.buffer_address & ~UINT32_C(3)) > UINT32_MAX - 4u))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    if (direction == T425_CHANNEL_DMA_RX &&
        t425_channel_admission_21_is_compat_input(&core->channel_admission)) {
        t425_channel_dma_handoff_28_status_t compat_status =
            channel_dma_handoff_prepare_compat_rx_28(
                core, address, index, lane_index, &request, plan);
        if (compat_status != T425_CHANNEL_DMA_HANDOFF_28_PREPARED)
            return compat_status;
        plan->magic = T425_CHANNEL_DMA_HANDOFF_28_MAGIC;
        plan->architectural_channel = address;
        plan->normalized_ptr_reg = request.buffer_address & ~UINT32_C(3);
        plan->channel_index = index;
        plan->direction = direction;
        plan->lane = lane_index;
        plan->request_snapshot = request;
        memcpy(plan->admission_snapshot, entry, sizeof(*entry));
        memcpy(plan->lane_snapshot, lane, sizeof(*lane));
        memcpy(plan->bank_snapshot, &core->t425_channel_dma,
               sizeof(core->t425_channel_dma));
        memcpy(plan->shift_snapshot, &core->channel_dma_shift,
               sizeof(core->channel_dma_shift));
        plan->prepared = 1u;
        return T425_CHANNEL_DMA_HANDOFF_28_PREPARED;
    }

    if (direction == T425_CHANNEL_DMA_RX) {
        const t425_channel_dma_05_rx_state_t *state =
            &core->channel_dma_shift.rx[lane_index];
        if (state->configured || state->active)
            return T425_CHANNEL_DMA_HANDOFF_28_BUSY;
    } else {
        const t425_channel_dma_05_tx_state_t *state =
            &core->channel_dma_shift.tx[lane_index];
        if (state->configured || state->active ||
            state->stage != T425_CHANNEL_DMA_05_TX_IDLE)
            return T425_CHANNEL_DMA_HANDOFF_28_BUSY;
    }

    memcpy(&bank_candidate, &core->t425_channel_dma,
           sizeof(bank_candidate));
    if (direction == T425_CHANNEL_DMA_RX)
        bank_candidate.rx[lane_index].ptr_reg =
            request.buffer_address & ~UINT32_C(3);
    else
        bank_candidate.tx[lane_index].ptr_reg =
            request.buffer_address & ~UINT32_C(3);
    memcpy(&memory_candidate, &core->channel_dma_memory,
           sizeof(memory_candidate));
    memory_candidate.bank = &bank_candidate;
    memcpy(&plan->shift_candidate, &core->channel_dma_shift,
           sizeof(core->channel_dma_shift));
    plan->shift_candidate.bank = &bank_candidate;
    plan->shift_candidate.memory = &memory_candidate;
    if (direction == T425_CHANNEL_DMA_RX) {
        begin_status = t425_channel_dma_shift_05_rx_begin(
            &plan->shift_candidate, lane_index, request.buffer_address,
            request.length, request.request_id);
    } else {
        begin_status = t425_channel_dma_shift_05_tx_begin(
            &plan->shift_candidate, lane_index, request.buffer_address,
            request.length, request.request_id);
    }
    if (begin_status == T425_CHANNEL_DMA_05_BUSY)
        return T425_CHANNEL_DMA_HANDOFF_28_BUSY;
    if (begin_status != T425_CHANNEL_DMA_05_OK ||
        !t425_channel_dma_shift_05_invariants(
            &plan->shift_candidate,
            (t425_channel_dma_direction_t)direction, lane_index))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    /* The token retains candidate slot state, never stack-local pointers. */
    plan->shift_candidate.bank = NULL;
    plan->shift_candidate.memory = NULL;
    memcpy(&admission_candidate, &core->channel_admission,
           sizeof(admission_candidate));
    if (!t425_channel_admission_21_report_begin(
            &admission_candidate, address, request.request_id,
            T425_CHANNEL_ADMISSION_21_ACCEPTED))
        return T425_CHANNEL_DMA_HANDOFF_28_INVALID;

    plan->magic = T425_CHANNEL_DMA_HANDOFF_28_MAGIC;
    plan->architectural_channel = address;
    plan->normalized_ptr_reg = request.buffer_address & ~UINT32_C(3);
    plan->channel_index = index;
    plan->direction = direction;
    plan->lane = lane_index;
    plan->request_snapshot = request;
    memcpy(&plan->admission_candidate, &admission_candidate.channel[index],
           sizeof(plan->admission_candidate));
    memcpy(plan->admission_snapshot, entry, sizeof(*entry));
    memcpy(plan->lane_snapshot, lane, sizeof(*lane));
    memcpy(plan->bank_snapshot, &core->t425_channel_dma,
           sizeof(core->t425_channel_dma));
    memcpy(plan->shift_snapshot, &core->channel_dma_shift,
           sizeof(core->channel_dma_shift));
    plan->prepared = 1u;
    return T425_CHANNEL_DMA_HANDOFF_28_PREPARED;
}

bool t425_core_channel_dma_handoff_28_commit(
    t425_core_t *core, t425_channel_dma_handoff_28_plan_t *plan)
{
    t425_channel_admission_request_21_t current_request = {0};
    uint8_t index, direction, lane_index;
    const t425_channel_admission_entry_21_t *entry;
    const t425_channel_dma_lane_t *lane;

    if (plan != NULL && plan->prepared != 0u &&
        plan->rx_profile_snapshot ==
            T425_CHANNEL_RX_PROFILE_COMPAT_INPUT_FROZEN) {
        const t425_channel_admission_entry_21_t *compat_entry;
        const t425_channel_dma_lane_t *compat_lane;
        t425_channel_admission_request_21_t compat_request = {0};
        t425_channel_compat_rx_adapter_07_t *adapter = plan->compat_adapter;
        uint8_t compat_index, compat_direction, compat_lane_index;
        if (plan->magic != T425_CHANNEL_DMA_HANDOFF_28_MAGIC ||
            !channel_dma_handoff_core_ready_28(core) || adapter == NULL ||
            core->channel_compat_profile_08_context != adapter ||
            !t425_channel_admission_21_is_compat_input(
                &core->channel_admission) ||
            !channel_dma_handoff_resolve_28(plan->architectural_channel,
                &compat_index, &compat_direction, &compat_lane_index) ||
            compat_index != plan->channel_index ||
            compat_direction != T425_CHANNEL_DMA_RX ||
            compat_lane_index != plan->lane ||
            adapter->input == NULL ||
            memcmp(&core->channel_admission.channel[compat_index],
                   plan->admission_snapshot,
                   sizeof(core->channel_admission.channel[compat_index])) != 0 ||
            memcmp(&core->t425_channel_dma.rx[compat_lane_index],
                   plan->lane_snapshot,
                   sizeof(core->t425_channel_dma.rx[compat_lane_index])) != 0 ||
            memcmp(&core->t425_channel_dma, plan->bank_snapshot,
                   sizeof(core->t425_channel_dma)) != 0 ||
            memcmp(&core->channel_dma_shift, plan->shift_snapshot,
                   sizeof(core->channel_dma_shift)) != 0 ||
            memcmp(&core->channel_dma_rx_adapter_10,
                   plan->legacy_adapter_snapshot,
                   sizeof(core->channel_dma_rx_adapter_10)) != 0 ||
            memcmp(adapter->input, plan->compat_input_snapshot,
                   sizeof(*adapter->input)) != 0 ||
            memcmp(&adapter->endpoint[plan->request_snapshot.endpoint],
                   plan->compat_endpoint_snapshot,
                   sizeof(adapter->endpoint[plan->request_snapshot.endpoint])) != 0 ||
            !t425_channel_admission_21_get_request(
                &core->channel_admission, &core->t425_channel_dma,
                plan->architectural_channel, &compat_request) ||
            !channel_dma_handoff_same_request_28(
                &compat_request, &plan->request_snapshot) ||
            plan->normalized_ptr_reg !=
                (plan->request_snapshot.buffer_address & ~UINT32_C(3)))
            goto reject_before_commit;
        compat_entry = &core->channel_admission.channel[compat_index];
        compat_lane = &core->t425_channel_dma.rx[compat_lane_index];
        {
            t425_channel_admission_entry_21_t expected_entry = *compat_entry;
            t425_channel_compat_input_key_t expected_key;
            t425_channel_compat_input_t expected_input;
            expected_entry.state = T425_CHANNEL_ADMISSION_21_ACTIVE;
            expected_entry.owner_is_f05 = 0u;
            expected_key = plan->compat_endpoint_candidate.key;
            expected_input = *adapter->input;
            if (!t425_channel_compat_input_capture_after_pushes_at_offset(
                    &expected_input, &expected_key.route,
                    expected_key.request_id, expected_key.generation, 2u,
                    plan->request_snapshot.initial_count_reg,
                    plan->normalized_ptr_reg,
                    plan->admission_candidate.initial_dbuff_reg,
                    (uint8_t)(plan->request_snapshot.buffer_address & 3u), true) ||
                memcmp(&expected_input, &plan->compat_input_candidate,
                       sizeof(expected_input)) != 0 ||
                memcmp(&expected_entry, &plan->admission_candidate,
                       sizeof(expected_entry)) != 0)
                goto reject_before_commit;
        }
        if (compat_lane->ptr_reg != plan->request_snapshot.buffer_address)
            goto reject_before_commit;

        /* All rejection paths end above this point. Only infallible stores
         * remain; ACTIVE is the final observable publication. */
        plan->prepared = 0u;
        *adapter->input = plan->compat_input_candidate;
        adapter->endpoint[plan->request_snapshot.endpoint] =
            plan->compat_endpoint_candidate;
        core->t425_channel_dma.rx[compat_lane_index].ptr_reg =
            plan->normalized_ptr_reg;
        core->channel_admission.channel[compat_index] =
            plan->admission_candidate;
        return true;
    }

    if (plan == NULL || plan->prepared == 0u ||
        plan->magic != T425_CHANNEL_DMA_HANDOFF_28_MAGIC ||
        !channel_dma_handoff_core_ready_28(core) ||
        !channel_dma_handoff_resolve_28(
            plan->architectural_channel, &index, &direction, &lane_index) ||
        index != plan->channel_index || direction != plan->direction ||
        lane_index != plan->lane)
        goto reject_before_commit;

    entry = &core->channel_admission.channel[index];
    lane = channel_dma_handoff_const_lane_28(
        &core->t425_channel_dma, direction, lane_index);
    if (lane == NULL ||
        memcmp(entry, plan->admission_snapshot, sizeof(*entry)) != 0 ||
        memcmp(lane, plan->lane_snapshot, sizeof(*lane)) != 0 ||
        memcmp(&core->t425_channel_dma, plan->bank_snapshot,
               sizeof(core->t425_channel_dma)) != 0 ||
        memcmp(&core->channel_dma_shift, plan->shift_snapshot,
               sizeof(core->channel_dma_shift)) != 0 ||
        !t425_channel_admission_21_get_request(
            &core->channel_admission, &core->t425_channel_dma,
            plan->architectural_channel, &current_request) ||
        !channel_dma_handoff_same_request_28(
            &current_request, &plan->request_snapshot) ||
        plan->normalized_ptr_reg !=
            (plan->request_snapshot.buffer_address & ~UINT32_C(3)))
        goto reject_before_commit;

    {
        t425_channel_admission_entry_21_t expected_entry;
        memcpy(&expected_entry, entry, sizeof(expected_entry));
        expected_entry.state = T425_CHANNEL_ADMISSION_21_ACTIVE;
        expected_entry.owner_is_f05 = 1u;
        if (memcmp(&expected_entry, &plan->admission_candidate,
                   sizeof(expected_entry)) != 0)
            goto reject_before_commit;
    }
    if (direction == T425_CHANNEL_DMA_RX) {
        t425_channel_dma_05_rx_state_t expected_state;
        memset(&expected_state, 0, sizeof(expected_state));
        expected_state.configured = true;
        expected_state.active = true;
        expected_state.buffer_start_address = plan->request_snapshot.buffer_address;
        expected_state.initial_count_reg = plan->request_snapshot.initial_count_reg;
        expected_state.next_request_id = plan->request_snapshot.request_id;
        expected_state.first_word_offset =
            (uint8_t)(plan->request_snapshot.buffer_address & 3u);
        if (memcmp(&expected_state, &plan->shift_candidate.rx[lane_index],
                   sizeof(expected_state)) != 0)
            goto reject_before_commit;
    } else {
        t425_channel_dma_05_tx_state_t expected_state;
        memset(&expected_state, 0, sizeof(expected_state));
        expected_state.configured = true;
        expected_state.active = true;
        expected_state.buffer_start_address = plan->request_snapshot.buffer_address;
        expected_state.initial_count_reg = plan->request_snapshot.initial_count_reg;
        expected_state.next_request_id = plan->request_snapshot.request_id;
        expected_state.first_word_offset =
            (uint8_t)(plan->request_snapshot.buffer_address & 3u);
        expected_state.stage = T425_CHANNEL_DMA_05_TX_IDLE;
        if (memcmp(&expected_state, &plan->shift_candidate.tx[lane_index],
                   sizeof(expected_state)) != 0)
            goto reject_before_commit;
    }

    /* No calls, validation, allocation, callback, or failure path below. */
    plan->prepared = 0u;
    if (direction == T425_CHANNEL_DMA_RX) {
        core->channel_dma_shift.rx[lane_index] =
            plan->shift_candidate.rx[lane_index];
        core->t425_channel_dma.rx[lane_index].ptr_reg =
            plan->normalized_ptr_reg;
    } else {
        core->channel_dma_shift.tx[lane_index] =
            plan->shift_candidate.tx[lane_index];
        core->t425_channel_dma.tx[lane_index].ptr_reg =
            plan->normalized_ptr_reg;
    }
    /* ACTIVE/ownership is the final observable live-state publication. */
    core->channel_admission.channel[index] = plan->admission_candidate;
    return true;

reject_before_commit:
    if (plan != NULL)
        plan->prepared = 0u;
    return false;
}

static void channel_dma_handoff_report_invalid_28(
    t425_core_t *core, uint32_t address, uint8_t index)
{
    t425_channel_admission_21_t candidate;
    uint32_t request_id = core->channel_admission.channel[index].request_id;
    memcpy(&candidate, &core->channel_admission, sizeof(candidate));
    if (t425_channel_admission_21_is_compat_input(&core->channel_admission)) {
        if (t425_channel_admission_21_report_invalid_compat_input(
                &candidate, address, request_id))
            core->channel_admission.channel[index] = candidate.channel[index];
    } else if (t425_channel_admission_21_report_begin(
                   &candidate, address, request_id,
                   T425_CHANNEL_ADMISSION_21_INVALID)) {
        core->channel_admission.channel[index] = candidate.channel[index];
    }
}

static bool channel_dma_rx_adapter_service_active_10(t425_core_t *core)
{
    bool progressed = false;
    uint32_t polled_endpoints = 0u;

    if (core->phy == NULL || !core->phy->attached)
        return false;
    if (!core->channel_dma_rx_adapter_10_initialized) {
        if (!t425_channel_dma_rx_adapter_10_init(
                &core->channel_dma_rx_adapter_10, core->phy,
                &core->channel_dma_shift))
            return false;
        core->channel_dma_rx_adapter_10_initialized = 1u;
    }

    /* Only sessions ACTIVE before this pass are eligible. REQUEST_PENDING
     * sessions are handed to F05 below and first polled on the next service
     * call, which occurs after the next microcycle. */
    for (uint8_t i = 0u; i < T425_CHANNEL_DMA_SLOT_COUNT; ++i) {
        const t425_channel_admission_entry_21_t *entry =
            &core->channel_admission.channel[i];
        t425_channel_admission_request_21_t request;
        uint32_t address = channel_dma_handoff_addresses_28[i];
        if (entry->state != T425_CHANNEL_ADMISSION_21_ACTIVE ||
            entry->owner_is_f05 == 0u ||
            entry->direction != T425_CHANNEL_DMA_RX ||
            entry->endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
            (polled_endpoints & (UINT32_C(1) << entry->endpoint)) != 0u)
            continue;
        if (!t425_core_channel_admission_get_active(
                core, address, entry->request_id,
                entry->operation_generation, entry->endpoint,
                entry->physical_link, &request))
            continue;
        if (t425_channel_dma_rx_adapter_10_adopt_existing(
                &core->channel_dma_rx_adapter_10, &request) !=
            T425_CHANNEL_DMA_05_OK)
            continue;

        polled_endpoints |= UINT32_C(1) << entry->endpoint;
        t425_channel_dma_rx_adapter_10_result_t result =
            t425_channel_dma_rx_adapter_10_poll_endpoint(
                &core->channel_dma_rx_adapter_10, entry->endpoint);
        if (result == T425_CHANNEL_DMA_RX_ADAPTER_10_ACCEPTED ||
            result == T425_CHANNEL_DMA_RX_ADAPTER_10_ACCEPTED_RETRY ||
            result == T425_CHANNEL_DMA_RX_ADAPTER_10_DRAINED)
            progressed = true;
    }
    return progressed;
}



bool t425_core_channel_dma_service(t425_core_t *core)
{
    bool accepted_any;
    if (!channel_dma_handoff_core_ready_28(core))
        return false;
    if (t425_channel_admission_21_is_compat_input(&core->channel_admission) &&
        core->channel_compat_profile_08_service != NULL) {
        accepted_any = core->channel_compat_profile_08_service(
            core->channel_compat_profile_08_context, core);
    } else {
        accepted_any = channel_dma_rx_adapter_service_active_10(core);
    }
    for (uint8_t i = 0u; i < T425_CHANNEL_ADMISSION_21_CHANNELS; ++i) {
        t425_channel_admission_entry_21_t *entry =
            &core->channel_admission.channel[i];
        t425_channel_dma_handoff_28_plan_t plan;
        t425_channel_dma_handoff_28_status_t status;
        uint32_t address = channel_dma_handoff_addresses_28[i];
        if (entry->state != T425_CHANNEL_ADMISSION_21_REQUEST_PENDING)
            continue;
        if (address == UINT32_C(0x80000000) && entry->request_id == 144u &&
            entry->operation_generation == 144u &&
            entry->state == T425_CHANNEL_ADMISSION_21_REQUEST_PENDING)
            {}
        status = t425_core_channel_dma_handoff_28_prepare(core, address, &plan);
        if (address == UINT32_C(0x80000000) && entry->request_id == 144u &&
            entry->operation_generation == 144u &&
            entry->state == T425_CHANNEL_ADMISSION_21_REQUEST_PENDING)
            {}
        if (status == T425_CHANNEL_DMA_HANDOFF_28_BUSY)
            continue;
        if (status == T425_CHANNEL_DMA_HANDOFF_28_INVALID) {
            channel_dma_handoff_report_invalid_28(core, address, i);
            continue;
        }
        if (t425_core_channel_dma_handoff_28_commit(core, &plan))
            accepted_any = true;
    }
    return accepted_any;
}

bool t425_core_channel_admission_configure_route(
    t425_core_t *core, uint32_t architectural_channel,
    uint8_t endpoint, uint8_t physical_link)
{
    if (core == NULL || endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        physical_link >= T425_CORE_PHY_MAX_ENDPOINTS)
        return false;
    return t425_channel_admission_21_configure_route(
        &core->channel_admission, architectural_channel, endpoint, physical_link);
}

bool t425_core_channel_admission_get_request(
    const t425_core_t *core, uint32_t architectural_channel,
    t425_channel_admission_request_21_t *request)
{
    return core != NULL && t425_channel_admission_21_get_request(
        &core->channel_admission, &core->t425_channel_dma,
        architectural_channel, request);
}

bool t425_core_channel_admission_get_active_compat_input(
    const t425_core_t *core, uint32_t architectural_channel,
    uint32_t expected_request_id, uint32_t expected_operation_generation,
    uint8_t expected_endpoint, uint8_t expected_physical_link,
    t425_channel_admission_request_21_t *request)
{
    uint8_t index, direction, lane_index;
    const t425_channel_admission_entry_21_t *entry;
    t425_channel_admission_request_21_t candidate;
    if (core == NULL || request == NULL || expected_request_id == 0u ||
        expected_operation_generation == 0u ||
        expected_endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        expected_physical_link >= T425_CORE_PHY_MAX_ENDPOINTS ||
        !t425_channel_admission_21_is_compat_input(&core->channel_admission) ||
        !channel_dma_handoff_core_ready_28(core) ||
        !channel_dma_handoff_resolve_28(
            architectural_channel, &index, &direction, &lane_index) ||
        direction != T425_CHANNEL_DMA_RX)
        return false;
    entry = &core->channel_admission.channel[index];
    if (entry->state != T425_CHANNEL_ADMISSION_21_ACTIVE ||
        entry->owner_is_f05 != 0u || entry->enabled == 0u ||
        entry->descriptor_valid == 0u ||
        entry->channel_word_published == 0u || entry->route_valid == 0u ||
        entry->architectural_channel != architectural_channel ||
        entry->direction != T425_CHANNEL_DMA_RX || entry->lane != lane_index ||
        entry->request_id != expected_request_id ||
        entry->operation_generation != expected_operation_generation ||
        entry->endpoint != expected_endpoint ||
        entry->physical_link != expected_physical_link ||
        entry->length == 0u ||
        entry->initial_count_reg != (uint32_t)(0u - entry->length))
        return false;
    candidate.request_id = entry->request_id;
    candidate.operation_generation = entry->operation_generation;
    candidate.architectural_channel = entry->architectural_channel;
    candidate.buffer_address = entry->buffer_address;
    candidate.initial_count_reg = entry->initial_count_reg;
    candidate.length = entry->length;
    candidate.channel_word = entry->channel_word;
    candidate.direction = entry->direction;
    candidate.lane = entry->lane;
    candidate.endpoint = entry->endpoint;
    candidate.physical_link = entry->physical_link;
    *request = candidate;
    return true;
}

bool t425_core_channel_admission_get_active(
    const t425_core_t *core, uint32_t architectural_channel,
    uint32_t expected_request_id, uint32_t expected_operation_generation,
    uint8_t expected_endpoint, uint8_t expected_physical_link,
    t425_channel_admission_request_21_t *request)
{
    uint8_t index, direction, lane_index;
    const t425_channel_admission_entry_21_t *entry;
    const t425_channel_dma_lane_t *lane;
    t425_channel_admission_request_21_t candidate;
    bool session_matches;

    if (core == NULL || request == NULL || expected_request_id == 0u ||
        expected_operation_generation == 0u ||
        expected_endpoint >= T425_CORE_PHY_MAX_ENDPOINTS ||
        expected_physical_link >= T425_CORE_PHY_MAX_ENDPOINTS ||
        !channel_dma_handoff_core_ready_28(core) ||
        !channel_dma_handoff_resolve_28(
            architectural_channel, &index, &direction, &lane_index))
        return false;

    entry = &core->channel_admission.channel[index];
    lane = channel_dma_handoff_const_lane_28(
        &core->t425_channel_dma, direction, lane_index);
    if (lane == NULL ||
        entry->state != T425_CHANNEL_ADMISSION_21_ACTIVE ||
        entry->owner_is_f05 == 0u || entry->enabled == 0u ||
        entry->descriptor_valid == 0u ||
        entry->channel_word_published == 0u || entry->route_valid == 0u ||
        entry->architectural_channel != architectural_channel ||
        entry->direction != direction || entry->lane != lane_index ||
        entry->request_id != expected_request_id ||
        entry->operation_generation != expected_operation_generation ||
        entry->endpoint != expected_endpoint ||
        entry->physical_link != expected_physical_link ||
        entry->length == 0u ||
        entry->initial_count_reg != (uint32_t)(0u - entry->length) ||
        (lane->ptr_reg & 3u) != 0u)
        return false;

    if (direction == T425_CHANNEL_DMA_RX) {
        const t425_channel_dma_05_rx_state_t *rx =
            &core->channel_dma_shift.rx[lane_index];
        session_matches = rx->configured && rx->active &&
            rx->buffer_start_address == entry->buffer_address &&
            rx->initial_count_reg == entry->initial_count_reg &&
            rx->next_request_id == entry->request_id &&
            rx->first_word_offset == (uint8_t)(entry->buffer_address & 3u);
    } else if (direction == T425_CHANNEL_DMA_TX) {
        const t425_channel_dma_05_tx_state_t *tx =
            &core->channel_dma_shift.tx[lane_index];
        session_matches = tx->configured && tx->active &&
            tx->buffer_start_address == entry->buffer_address &&
            tx->initial_count_reg == entry->initial_count_reg &&
            tx->next_request_id == entry->request_id &&
            tx->first_word_offset == (uint8_t)(entry->buffer_address & 3u);
    } else {
        return false;
    }
    if (!session_matches)
        return false;

    candidate.request_id = entry->request_id;
    candidate.operation_generation = entry->operation_generation;
    candidate.architectural_channel = entry->architectural_channel;
    candidate.buffer_address = entry->buffer_address;
    candidate.initial_count_reg = entry->initial_count_reg;
    candidate.length = entry->length;
    candidate.channel_word = entry->channel_word;
    candidate.direction = entry->direction;
    candidate.lane = entry->lane;
    candidate.endpoint = entry->endpoint;
    candidate.physical_link = entry->physical_link;
    *request = candidate;
    return true;
}

bool t425_core_channel_admission_report_begin(
    t425_core_t *core, uint32_t architectural_channel, uint32_t request_id,
    t425_channel_admission_begin_result_21_t result)
{
    return core != NULL && t425_channel_admission_21_report_begin(
        &core->channel_admission, architectural_channel, request_id, result);
}

bool t425_core_channel_admission_report_complete_compat_input(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation)
{
    return core != NULL &&
        t425_channel_admission_21_report_complete_compat_input(
            &core->channel_admission, architectural_channel,
            operation_generation);
}

bool t425_core_channel_admission_retire_compat_input(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation)
{
    return core != NULL &&
        t425_channel_admission_21_retire_compat_input(
            &core->channel_admission, architectural_channel,
            operation_generation);
}

bool t425_core_channel_admission_report_complete(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation)
{
    return core != NULL && t425_channel_admission_21_report_complete(
        &core->channel_admission, architectural_channel, operation_generation);
}

bool t425_core_channel_admission_retire(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation)
{
    return core != NULL && t425_channel_admission_21_retire(
        &core->channel_admission, architectural_channel, operation_generation);
}

bool t425_core_channel_admission_abort(
    t425_core_t *core, uint32_t architectural_channel,
    uint32_t operation_generation)
{
    return core != NULL && t425_channel_admission_21_abort(
        &core->channel_admission, architectural_channel, operation_generation);
}

void t425_core_microentry(
    t425_core_t *core,
    uint8_t opcode)
{
    uint16_t mpc;

    if (!core)
        return;

    mpc = t425_microentry_from_instruction(
        opcode,
        core->cpu.Oreg,
        core->microentry_signal);

    t425_microsequencer_reset(
        &core->microsequencer,
        mpc);
}
