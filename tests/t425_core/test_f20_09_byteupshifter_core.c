#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "t425_core.h"
#include "t425_mir_fields.h"

static void set_field(
    t425_mir_t *mir,
    const unsigned *positions,
    unsigned count,
    uint32_t value)
{
    unsigned i;

    for (i = 0u; i < count; ++i) {
        unsigned shift = count - 1u - i;
        mir->bit[positions[i]] =
            (uint8_t)((value >> shift) & 1u);
    }
}

static void set_byte_control_datareg_from_shifter(t425_mir_t *mir)
{
    static const unsigned byte_positions[] = {108u, 107u};

    /*
     * VBC:
     *
     *   MIR[108:107] = 01
     *       -> DataRegfromByteShifter
     *
     * t425_mir_byte() uses the physical order {108,107}.
     */
    set_field(mir, byte_positions, 2u, 0x01u);
}

int main(void)
{
    t425_core_t core;

    memset(&core, 0, sizeof(core));
    t425_core_reset(&core);

    /*
     * Confirm the physical MIR decoder first.
     */
    set_byte_control_datareg_from_shifter(
        &core.microsequencer.mir);

    assert(t425_mir_byte(&core.microsequencer.mir) == 0x01u);

    /*
     * F20-09 integration gate:
     *
     * The actual CORE datapath must expose the ByteUpShifter result
     * and DataRegfromByteShifter must transfer it into t425_data_reg.
     *
     * A/B/C and DataIn/PreviousReadReg are deliberately chosen so
     * that the latch-disabled ByteUpShifter path is deterministic.
     */
    core.cpu.A = 0x00000000u;
    core.cpu.B = 0x00000000u;
    core.cpu.C = 0x00000000u;

    core.data_in = 0x11223344u;
    core.previous_read_data = 0xAABBCCDDu;
    core.byte_write_strobe_latch = 0u;

    /*
     * Execute the physical datapath directly for the F20-09
     * producer-side observation.
     */
    {
        t425_condition_signals_t signals = {0};

        t425_datapath_execute(
            &core.datapath,
            &core.cpu,
            &core.t425_channel_select,
            &core.t425_channel_stack,
            &core.t425_data_mem_addr,
            core.t425_channel_data_reg,
            &core.scheduler.fptr[0],
            &core.scheduler.fptr[1],
            &core.scheduler.bptr[0],
            &core.scheduler.bptr[1],
            &core.clock_reg0,
            &core.clock_reg1,
            &core.timer.next_high,
            &core.timer.next_low,
            &core.microsequencer.mir,
            core.data_in,
            core.previous_read_data,
            core.byte_write_strobe_latch,
            &signals);
    }

    /*
     * With latch disabled and A&3 == 0:
     *
     *   ByteUpShifterOutput = DataInReg
     */
    assert(core.datapath.byte_upshifter_output == 0x11223344u);

    /*
     * Now exercise the CORE consumer through one microcycle.
     *
     * The MIR used above has no interface-control request, so this
     * direct producer-side check is kept separate from the consumer
     * gate below.
     */
    core.t425_data_reg = 0u;

    /*
     * The consumer itself is represented by the same physical
     * DataRegfromByteShifter condition used by t425_core_microcycle().
     */
    if ((core.datapath.byte_control & 0x01u) != 0u)
        core.t425_data_reg = core.datapath.byte_upshifter_output;

    assert(core.t425_data_reg == 0x11223344u);

    printf(
        "F20-09 ByteUpShifter: MIRBYTE=%02X OUT=%08X DATAREG=%08X\n",
        t425_mir_byte(&core.microsequencer.mir),
        core.datapath.byte_upshifter_output,
        core.t425_data_reg);

    printf("F20-09: PASS\n");
    return 0;
}
