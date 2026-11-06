#ifndef T425_DATAPATH_H
#define T425_DATAPATH_H

#include <stdint.h>

#include "t425_cpu.h"
#include "t425_mir.h"
#include "t425_microsequencer.h"
#include "t425_incrementor.h"
#include "t425_channel_dma_foundation_01.h"

typedef struct {
    uint32_t xbus;
    uint32_t ybus;
    uint32_t zbus;

    uint8_t alu_carryout;
    uint8_t alu_overflow;
    uint8_t alu_prop_all1;
    uint8_t zbus_sub_less_than0;

    uint32_t constbox;
    uint8_t byte_control;
    uint8_t ybus_byte_select;

    /* F20: ByteUpShifter output. */
    uint32_t byte_upshifter_output;
    t425_incrementor_result_t incrementor;

    /* Non-owning reference set by the CORE for the MIR ChannelStack target. */
    t425_channel_dma_bank_t *channel_stack_bank;
} t425_datapath_t;

void t425_datapath_reset(t425_datapath_t *datapath);

void t425_datapath_execute(
    t425_datapath_t *datapath,
    t425_cpu_state_t *cpu,
    uint32_t *channel_select,
    uint32_t *channel_stack,
    uint32_t *data_mem_addr,
    uint8_t *data_mem_addr_from_wdesc,
    uint32_t channel_data,
    uint32_t *fptr0,
    uint32_t *fptr1,
    uint32_t *bptr0,
    uint32_t *bptr1,
    uint32_t *clock_reg0,
    uint32_t *clock_reg1,
    uint32_t *tnext_reg0,
    uint32_t *tnext_reg1,
    const t425_mir_t *mir,
    uint32_t data_in,
    uint32_t data_in_reg,
    uint32_t previous_read_data,
    uint8_t byte_write_strobe_latch,
    uint16_t current_mpc,
    t425_condition_signals_t *signals);

#endif
