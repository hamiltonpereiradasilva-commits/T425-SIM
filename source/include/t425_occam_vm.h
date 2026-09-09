#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_occam.h"
#include "t425_channels.h"
#include "t425_timer.h"

#define T425_OCCAM_VARS 16
typedef enum { OVM_READY=0, OVM_WAIT_IN, OVM_WAIT_OUT, OVM_WAIT_TIMER, OVM_DONE, OVM_ERROR } t425_ovm_state_t;

typedef struct {
    uint32_t pid;
    uint16_t pc;
    uint32_t vars[T425_OCCAM_VARS];
    t425_ovm_state_t state;
    uint32_t wait_channel;
    uint32_t wait_timer;
    const t425_occam_program_t *program;
} t425_occam_vm_t;

void t425_occam_vm_init(t425_occam_vm_t *vm,const t425_occam_program_t *p);
int t425_occam_vm_step(t425_occam_vm_t *vm,t425_channel_t *channels,t425_timer_t *timer);
