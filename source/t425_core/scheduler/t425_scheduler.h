#ifndef T425_CORE_SCHEDULER_H
#define T425_CORE_SCHEDULER_H

#include <stdint.h>

#include "t425_memory.h"

#define T425_NOT_PROCESS 0x80000000u

#define T425_PRIORITY_HIGH 0u
#define T425_PRIORITY_LOW  1u

/*
 * Architectural runnable-process queue state.
 *
 * FPtr[p] points to the first runnable process descriptor of priority p.
 * BPtr[p] points to the last runnable process descriptor of priority p.
 *
 * The link field of a process resides at Wptr - 8.
 */
typedef struct
{
    uint32_t fptr[2];
    uint32_t bptr[2];

    /*
     * Simulator metadata retained during the F08.2 migration.
     * It is not a replacement for FPtr/BPtr.
     */
    uint32_t current_wdesc;
    uint8_t running;
} t425_scheduler_state_t;

void t425_scheduler_reset(t425_scheduler_state_t *scheduler);

int t425_scheduler_enqueue(
    t425_scheduler_state_t *scheduler,
    t425_memory_t *memory,
    uint32_t wdesc);

int t425_scheduler_dequeue(
    t425_scheduler_state_t *scheduler,
    t425_memory_t *memory,
    uint32_t *wdesc,
    uint32_t *iptr);

#endif
