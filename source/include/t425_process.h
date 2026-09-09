#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_vm.h"

#ifndef T425_WORKSPACE_WORDS
#define T425_WORKSPACE_WORDS 64
#endif

/* Extended process descriptor used by the runtime layer.
   The VM's t425_process_t remains the architectural process record. */
typedef struct {
    uint32_t pid;
    uint32_t pc;
    uint32_t w;
    uint32_t sp;
    uint32_t priority;
    uint32_t workspace[T425_WORKSPACE_WORDS];
    t425_proc_state_t state;
} t425_runtime_process_t;

void t425_process_table_init(t425_runtime_process_t *p);
bool t425_process_start(t425_runtime_process_t *p,uint32_t pid,uint32_t pc,uint32_t priority);
bool t425_process_stop(t425_runtime_process_t *p,uint32_t pid);
bool t425_process_run(t425_runtime_process_t *p,uint32_t pid);
bool t425_process_end(t425_runtime_process_t *p,uint32_t pid);
