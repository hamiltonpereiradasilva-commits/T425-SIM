#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "t425_runtime.h"
#include "t425_vm.h"

typedef enum {
    T425_SCHED_INIT = 0,
    T425_SCHED_READY,
    T425_SCHED_RUNNING,
    T425_SCHED_FAULT
} t425_sched_state_t;

typedef struct {
    t425_sched_state_t state;

    uint32_t cycles;
    uint32_t rx_events;
    uint32_t tx_events;
    uint32_t timer_events;
    uint32_t fault_events;

    /*
     * Scheduler arquitetural.
     *
     * O PID continua temporariamente como mecanismo interno
     * de gerenciamento durante a transição para processos
     * baseados integralmente em workspace.
     */
    uint32_t high_queue[T425_MAX_PROCESSES];
    uint32_t low_queue[T425_MAX_PROCESSES];

    uint32_t high_head;
    uint32_t high_tail;
    uint32_t high_count;

    uint32_t low_head;
    uint32_t low_tail;
    uint32_t low_count;

    uint32_t current_pid;

    uint32_t process_switches;

} t425_scheduler_t;

bool t425_scheduler_init(t425_scheduler_t *s);

bool t425_scheduler_start(t425_scheduler_t *s);

bool t425_scheduler_enqueue(
    t425_scheduler_t *s,
    uint32_t pid,
    uint32_t priority
);

bool t425_scheduler_dequeue(
    t425_scheduler_t *s,
    uint32_t *pid
);

bool t425_scheduler_step(
    t425_scheduler_t *s,
    t425_runtime_t *rt
);

bool t425_scheduler_fault(
    t425_scheduler_t *s
);
