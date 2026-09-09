#pragma once
#include "t425_occam_parallel.h"
#include "t425_occam_vm.h"
#include "t425_timer.h"
#include "t425_channels.h"

typedef struct {
    t425_parallel_image_t image;
    t425_occam_program_t program[T425_MAX_PROCESSES];
    t425_occam_vm_t vm[T425_MAX_PROCESSES];
    t425_channel_t channels[T425_CHANNELS];
    t425_timer_t timer;
    uint16_t rr;
} t425_parallel_vm_t;

bool t425_parallel_vm_init(t425_parallel_vm_t *v,const t425_ast_t *ast);
int t425_parallel_vm_run(t425_parallel_vm_t *v,uint32_t max_steps);
