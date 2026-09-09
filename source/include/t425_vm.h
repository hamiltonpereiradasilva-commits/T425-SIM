#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_config.h"

typedef struct {
    uint32_t A, B, C;
    uint32_t W, I, PC;
    uint32_t OPR;
    uint8_t error, halted, prefix_valid, prefix_negative;
} t425_cpu_t;

typedef enum {
    PROC_FREE=0, PROC_READY, PROC_RUNNING, PROC_WAIT_CHAN,
    PROC_WAIT_TIMER, PROC_STOPPED, PROC_TERMINATED
} t425_proc_state_t;

typedef struct {
    uint32_t pid, pc, w, priority;
    t425_proc_state_t state;
} t425_process_t;

typedef struct {
    t425_cpu_t cpu;
    uint8_t ram[T425_RAM_BYTES];
    t425_process_t proc[T425_MAX_PROCESSES];
    uint32_t current_pid;
    uint32_t current_priority;
    uint64_t instructions;
    uint32_t timer;
    uint8_t analyse;
} t425_vm_t;

void t425_vm_reset(t425_vm_t *vm);
bool t425_vm_load(t425_vm_t *vm, uint32_t addr, const uint8_t *data, uint32_t len);
bool t425_vm_read_mem(t425_vm_t *vm, uint32_t addr, uint8_t *out, uint32_t len);
bool t425_vm_write_mem(t425_vm_t *vm, uint32_t addr, const uint8_t *data, uint32_t len);
bool t425_vm_step(t425_vm_t *vm);
void t425_vm_run_slice(t425_vm_t *vm, uint32_t budget);
void t425_vm_get_regs(const t425_vm_t *vm, uint32_t out[6]);
void t425_vm_tick(t425_vm_t *vm);
void t425_vm_halt(t425_vm_t *vm);
void t425_vm_run(t425_vm_t *vm);
