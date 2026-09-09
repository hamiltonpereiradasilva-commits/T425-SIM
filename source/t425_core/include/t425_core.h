#ifndef T425_CORE_H
#define T425_CORE_H

#include <stdint.h>

#include "t425_cpu.h"
#include "t425_memory.h"

typedef struct
{
    t425_cpu_state_t cpu;
    t425_memory_t memory;

    uint64_t cycles;
    uint64_t instructions;
} t425_core_t;

void t425_core_reset(t425_core_t *core);

#endif
