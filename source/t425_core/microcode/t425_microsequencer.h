#ifndef T425_MICROSEQUENCER_H
#define T425_MICROSEQUENCER_H

#include <stdint.h>
#include "t425_mir.h"

typedef struct
{
    uint8_t rom_feedback0;
    uint8_t not_carryout;
    uint8_t not_zbus_sub_less_than0;
    uint8_t not_alu_prop_all1;
    uint8_t overflow;
    uint8_t not_inc_oreg_carryout;
    uint8_t not_inc_dreg_carryout;
    uint8_t not_pr_in_analyse;

    uint8_t rom_feedback1;
    uint8_t div_step0;
    uint8_t div_step1;
    uint8_t a_slave0;
    uint8_t b_slave0;
    uint8_t b_slave1;
    uint8_t b_slave_msb;
    uint8_t c_slave_msb;
    uint8_t not_two_reads_to_start;
    uint8_t proc_priority;
    uint8_t boot_from_rom_not_links;
    uint8_t ch_or_timer_req;
    uint8_t ch_not_timer_req;
    uint8_t high_not_low_req;
    uint8_t channel_ready_not_run;
    uint8_t timeout_req;
} t425_condition_signals_t;

typedef struct
{
    uint16_t mpc;
    uint16_t next_base;
    uint16_t next_mpc;

    uint8_t cond0_select;
    uint8_t cond1_select;

    uint8_t cond0_selected;
    uint8_t cond1_selected;

    t425_mir_t mir;
} t425_microsequencer_t;

void t425_microsequencer_reset(
    t425_microsequencer_t *seq,
    uint16_t initial_mpc);

void t425_microsequencer_load_mir(
    t425_microsequencer_t *seq);

uint16_t t425_microsequencer_next_base(
    t425_microsequencer_t *seq);

uint8_t t425_microsequencer_select_cond0(
    const t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals);

uint8_t t425_microsequencer_select_cond1(
    const t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals);

uint16_t t425_microsequencer_next_mpc(
    t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals);

void t425_microsequencer_step(
    t425_microsequencer_t *seq,
    const t425_condition_signals_t *signals);

#endif /* T425_MICROSEQUENCER_H */
