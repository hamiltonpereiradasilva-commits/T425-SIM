#ifndef T425_CPU_H
#define T425_CPU_H

#include <stdint.h>

typedef struct
{
    uint32_t A;
    uint32_t B;
    uint32_t C;

    uint32_t Wptr;
    uint32_t Iptr;

    uint32_t Oreg;
    uint32_t Breg;
    uint32_t Creg;

    uint8_t error;
    uint8_t halt_on_error;
    uint8_t analyse;
    uint8_t halted;

    uint8_t j0_break;
    uint8_t timer_disabled_high;
    uint8_t timer_disabled_low;

    uint32_t devid;
} t425_cpu_state_t;

void t425_cpu_reset(t425_cpu_state_t *cpu);

#endif
