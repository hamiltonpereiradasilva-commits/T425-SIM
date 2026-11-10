#ifndef T425_TEST_F1_FIXTURE_COMPAT_H
#define T425_TEST_F1_FIXTURE_COMPAT_H

#include "t425_cpu.h"

/* Synchronize explicit fixture state before it enters the F1 model. */
static inline void t425_fixture_sync_slaves(t425_cpu_state_t *cpu)
{
    cpu->ASlave = cpu->A;
    cpu->BSlave = cpu->B;
    cpu->CSlave = cpu->C;
}

#endif
