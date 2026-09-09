#ifndef T425_CORE_FPU_H
#define T425_CORE_FPU_H

#include <stdint.h>

typedef struct
{
    uint8_t error;
    uint8_t rounding_mode;
} t425_fpu_state_t;

void t425_fpu_reset(
    t425_fpu_state_t *fpu);

#endif
