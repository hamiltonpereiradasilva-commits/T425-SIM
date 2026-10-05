#include "t425_core.h"
#include "t425_channel.h"
#include "t425_mir_fields.h"

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
    core->t425_data_mem_addr = 0u;
    core->t425_data_reg = 0u;
    core->previous_read_data = 0u;
    core->data_in = 0u;
    core->byte_write_strobe_latch = 0u;
    core->data_in_reg = 0u;
    core->two_reads_latch = 0u;
    core->clear_bwse_latch_after_write = 0u;
    core->t425_channel_data_reg = 0u;
    core->clock_reg0 = 0u;
    core->clock_reg1 = 0u;
    core->t425_channel_ready_not_run = 0u;
    core->t425_channel_run_req = 0u;
    core->t425_channel_status = 0u;
    core->t425_alt_enabled = 0u;
    core->phy = 0;
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

    return control != T425_CHANNEL_EMPTY;
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
    if (core->boot_state != T425_BOOT_RUNNING)
        core->rx_credit_rearm[link_id] = 1u;
    return 0;
}

void t425_core_microcycle(t425_core_t *core)
{
    t425_condition_signals_t signals;
    uint8_t interface_control;
    uint32_t data_reg_source;

    if (!core)
        return;

    /*
     * Capture the interface request belonging to the MIR that is
     * about to execute.  The request belongs to this microcycle;
     * therefore it must be sampled before the microsequencer loads
     * the next MIR.
     */
    interface_control =
        (uint8_t)(t425_mir_interface(
            &core->microsequencer.mir) & 0x7Fu);

    /*
     * DataInfromPrevReadReg exposes the result of the
     * PREVIOUS memory read. It must be sampled before a
     * MemRead in this same microword can replace the
     * previous-read latch.
     */
    if ((interface_control & 0x10u) != 0u)
        core->data_in = core->previous_read_data;

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
    data_reg_source = core->cpu.A;

    signals = (t425_condition_signals_t){0};

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
    if ((t425_mir_byte(&core->microsequencer.mir) & 0x02u) != 0u)
        core->byte_write_strobe_latch = 1u;

    /*
     * F20-10-05E — ByteUpShifter input is the physical DataInReg.
     *
     * DataInReg is the current memory-read result.
     * PreviousReadReg remains a separate temporal register and is
     * supplied independently below.
     *
     * DataInfromPrevReadReg continues to use core->data_in.
     */
    t425_datapath_execute(
        &core->datapath,
        &core->cpu,
        &core->t425_channel_select,
        &core->t425_channel_stack,
        &core->t425_data_mem_addr,
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
        core->data_in_reg,
        core->previous_read_data,
        core->byte_write_strobe_latch,
        &signals);

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
    {
        uint16_t channel_control =
            (uint16_t)(t425_mir_channel(
                &core->microsequencer.mir) & 0x07FFu);

        if ((channel_control & 0x080u) != 0u) {
            if (t425_channel_reset(
                    &core->memory,
                    core->t425_channel_select) != 0)
                core->cpu.error = 1u;
        }

        if ((channel_control & 0x040u) != 0u)
            core->t425_channel_ready_not_run = 0u;

        if ((channel_control & 0x002u) != 0u)
            core->t425_channel_run_req = 0u;
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

    /*
     * MemRead is sampled after the datapath has produced
     * DataMemAddrFromZbus for this microword.
     *
     * The result becomes previous_read_data and is therefore
     * available to DataInfromPrevReadReg in a later temporal
     * point.
     */
    if ((interface_control & 0x04u) != 0u) {
        uint32_t value = 0u;

        if (t425_memory_read_word(
                &core->memory,
                core->t425_data_mem_addr,
                &value) == 0) {
            /* Current DataInReg, physically distinct from
             * PreviousReadReg/DataInfromPrevReadReg. */
            core->data_in_reg = value;

            core->previous_read_data = value;

            /*
             * AregfromDataIn is simultaneous with MemRead in
             * LDNL/LDL: A receives the current read result.
             */
            if ((interface_control & 0x20u) != 0u)
                core->cpu.A = value;
        } else {
            core->cpu.error = 1u;
        }
    } else if ((interface_control & 0x20u) != 0u) {
        /*
         * AregfromDataIn without MemRead consumes the
         * DataIn value already present in this microcycle.
         */
        core->cpu.A = core->data_in;
    }

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
        if (t425_memory_write_word(
                &core->memory,
                core->t425_data_mem_addr,
                core->t425_data_reg) != 0) {
            core->cpu.error = 1u;
        } else if (core->clear_bwse_latch_after_write != 0u) {
            /* MemoryInterface consumes ClearBWSELatchAfterWrite. */
            core->byte_write_strobe_latch = 0u;
            core->clear_bwse_latch_after_write = 0u;
        }
    }

    t425_microsequencer_step(
        &core->microsequencer,
        &signals);

    /*
     * Next is the VBC Interface Control Request:
     *
     *     Next = #B1000000
     *
     * It requests return to the instruction-level FETCH stage.
     * It is not interpreted here as "MPC + 1"; the microsequencer
     * retains its already-established physical next-state behavior.
     */
    /*
     * T425 Interface Control: Next is bit 6 (0x40).
     * Next may coexist with other interface requests.
     * Therefore test the physical Next bit, not equality
     * with the isolated value 0x40.
     *
     * Physical CROM[1B2] has IF=0x5A:
     * Next | DataInfromPrevReadReg | MemWrite |
     * DataRegfromZbus.
     */
    core->next_step_instruction =
        ((interface_control & 0x40u) != 0u) ? 0u : 1u;
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
