#ifndef T425_CORE_TIMER_H
#define T425_CORE_TIMER_H

#include <stdint.h>
#include "t425_clock.h"

typedef struct
{
    uint32_t high;
    uint32_t low;
    uint32_t next_high;
    uint32_t next_low;

    /*
     * Architectural timer interrupt enable state.
     * TIMERENABLEH/TIMERENABLEL and TIMERDISABLEH/TIMERDISABLEL
     * consume these fields. Those instructions are outside F08.1-C.4.
     */
    uint8_t interrupt_high_enabled;
    uint8_t interrupt_low_enabled;

    /*
     * Private simulator metadata.
     * Indicates that the simulated timer clocks have been started
     * by STTIMER. This is not a T425 architectural register.
     */
    uint8_t running;

    uint32_t high_cycle_accum;
    uint32_t low_cycle_accum;

    /*
     * Private simulator metadata.
     * One bit per internal-RAM word identifies a process waiting
     * through TALTWT. It is not part of the T425 architectural state.
     */
    uint32_t alt_wait_mask[32];
} t425_timer_state_t;

void t425_timer_reset(t425_timer_state_t *timer);

void t425_timer_advance(
    t425_timer_state_t *timer,
    uint32_t processor_cycles);

uint32_t t425_timer_read_high(
    const t425_timer_state_t *timer);

uint32_t t425_timer_read_low(
    const t425_timer_state_t *timer);

#endif
