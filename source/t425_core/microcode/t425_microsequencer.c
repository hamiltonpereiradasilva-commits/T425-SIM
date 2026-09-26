#include "t425_microsequencer.h"

#include "t425_mir_fields.h"

#include <stddef.h>

void t425_microsequencer_reset(
    t425_microsequencer_t *seq,
    uint16_t initial_mpc)
{
    if (seq == NULL)
        return;

    seq->mpc = (uint16_t)(initial_mpc & 0x03FFu);
    seq->next_base = 0u;
    seq->next_mpc = seq->mpc;

    seq->cond0_select = 0u;
    seq->cond1_select = 0u;
    seq->cond0_selected = 0u;
    seq->cond1_selected = 0u;

    t425_mir_load(&seq->mir, seq->mpc);
}

void t425_microsequencer_load_mir(
    t425_microsequencer_t *seq)
{
    if (seq == NULL)
        return;

    seq->mpc &= 0x03FFu;
    t425_mir_load(&seq->mir, seq->mpc);
}

uint16_t t425_microsequencer_next_base(
    t425_microsequencer_t *seq)
{
    if (seq == NULL)
        return 0u;

    seq->next_base =
        (uint16_t)(t425_mir_next_field(&seq->mir) & 0x03FFu);

    return seq->next_base;
}

uint8_t t425_microsequencer_select_cond0(
    const t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals)
{
    if (seq == NULL || signals == NULL)
        return 0u;

    switch (seq->cond0_select & 0x07u)
    {
        case 0u:
            /*
             * ROMFeedback0 is not an external condition input.
             *
             * VBC:
             *   Microword[117:110,29,24] loads MIR
             *   ROMFeedback[0] = MIR[24]
             *
             * Therefore Cond0=ROMFeedback0 selects bit 0
             * of the physical 10-bit ROM feedback field.
             */
            return (uint8_t)(
                t425_mir_next_field(&seq->mir) & 0x01u
            );

        case 1u:
            return (uint8_t)(signals->not_carryout & 1u);

        case 2u:
            return (uint8_t)(signals->not_zbus_sub_less_than0 & 1u);

        case 3u:
            return (uint8_t)(signals->not_alu_prop_all1 & 1u);

        case 4u:
            return (uint8_t)(signals->overflow & 1u);

        case 5u:
            return (uint8_t)(signals->not_inc_oreg_carryout & 1u);

        case 6u:
            return (uint8_t)(signals->not_inc_dreg_carryout & 1u);

        case 7u:
            return (uint8_t)(signals->not_pr_in_analyse & 1u);

        default:
            return 0u;
    }
}

uint8_t t425_microsequencer_select_cond1(
    const t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals)
{
    if (seq == NULL || signals == NULL)
        return 0u;

    switch (seq->cond1_select & 0x0Fu)
    {
        case 0u:
            /*
             * ROMFeedback1 is the second bit of the physical
             * Microword[117:110,29,24] field.
             *
             * VBC:
             *   ROMFeedback[1] = MIR[29]
             *
             * t425_mir_next_field() reconstructs this field as:
             *   bit 0 -> MIR[24]
             *   bit 1 -> MIR[29]
             *   bits 2..9 -> MIR[110..117]
             */
            return (uint8_t)(
                (t425_mir_next_field(&seq->mir) >> 1) & 0x01u
            );

        case 1u:
            return (uint8_t)(signals->div_step0 & 1u);

        case 2u:
            return (uint8_t)(signals->div_step1 & 1u);

        case 3u:
            return (uint8_t)(signals->a_slave0 & 1u);

        case 4u:
            return (uint8_t)(signals->b_slave0 & 1u);

        case 5u:
            return (uint8_t)(signals->b_slave1 & 1u);

        case 6u:
            return (uint8_t)(signals->b_slave_msb & 1u);

        case 7u:
            return (uint8_t)(signals->c_slave_msb & 1u);

        case 8u:
            return (uint8_t)(signals->not_two_reads_to_start & 1u);

        case 9u:
            return (uint8_t)(signals->proc_priority & 1u);

        case 10u:
            return (uint8_t)(signals->boot_from_rom_not_links & 1u);

        case 11u:
            return (uint8_t)(signals->ch_or_timer_req & 1u);

        case 12u:
            return (uint8_t)(signals->ch_not_timer_req & 1u);

        case 13u:
            return (uint8_t)(signals->high_not_low_req & 1u);

        case 14u:
            return (uint8_t)(signals->channel_ready_not_run & 1u);

        case 15u:
            return (uint8_t)(signals->timeout_req & 1u);

        default:
            return 0u;
    }
}

uint16_t t425_microsequencer_next_mpc(
    t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals)
{
    uint16_t next_field;
    uint16_t next_high;

    if (seq == NULL || signals == NULL)
        return 0u;

    seq->cond0_select =
        (uint8_t)(t425_mir_cond0(&seq->mir) & 0x07u);

    seq->cond1_select =
        (uint8_t)(t425_mir_cond1(&seq->mir) & 0x0Fu);

    next_field =
        (uint16_t)(t425_mir_next_field(&seq->mir) & 0x03FFu);

    next_high =
        (uint16_t)(next_field & 0x03FCu);

    seq->cond0_selected =
        t425_microsequencer_select_cond0(seq, signals);

    seq->cond1_selected =
        t425_microsequencer_select_cond1(seq, signals);

    seq->next_base = next_field;

    /*
     * Physical NEXTMPC:
     *
     *   NEXTMPC[9:2] = MIR[117:110]
     *   NEXTMPC[1]   = Cond1Selected
     *   NEXTMPC[0]   = Cond0Selected
     *
     * The Interface Control Request Next is not a NEXTMPC
     * source. Interface requests are handled by the
     * instruction-level interface in t425_core.c.
     */
    seq->next_mpc =
        (uint16_t)(
            next_high |
            ((uint16_t)(seq->cond1_selected & 1u) << 1) |
            (uint16_t)(seq->cond0_selected & 1u));

    seq->next_mpc &= 0x03FFu;

    return seq->next_mpc;
}

void t425_microsequencer_step(
    t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals)
{
    if (seq == NULL || signals == NULL)
        return;

    t425_microsequencer_next_mpc(seq, signals);

    seq->mpc = (uint16_t)(seq->next_mpc & 0x03FFu);

    t425_microsequencer_load_mir(seq);
}
