#ifndef T425_CORE_H
#define T425_CORE_H

#include <stdint.h>

#include "t425_cpu.h"
#include "t425_memory.h"
#include "t425_scheduler.h"
#include "t425_timer.h"

typedef struct
{
    /* T425 VBC internal channel machinery. */
    uint32_t t425_channel_select;
    uint32_t t425_channel_stack;
    uint32_t t425_data_mem_addr;
    uint32_t t425_data_reg;

    t425_cpu_state_t cpu;
    t425_memory_t memory;
    t425_scheduler_state_t scheduler;
    t425_timer_state_t timer;
    uint64_t cycles;
    uint64_t instructions;
} t425_core_t;

#define T425_STEP_OK        0
#define T425_STEP_HALTED    1
#define T425_STEP_BADMEM   -1
#define T425_STEP_UNIMPL   -2
#define T425_STEP_BADOP    -3

void t425_core_reset(t425_core_t *core);

int t425_core_step(t425_core_t *core);

#endif
