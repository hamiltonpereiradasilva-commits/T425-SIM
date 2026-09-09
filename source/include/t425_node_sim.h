#pragma once
#include "t425_occam_vm.h"
typedef struct {
    t425_occam_vm_t vm[2];
    t425_channel_t channels[T425_CHANNELS];
    t425_timer_t timer;
} t425_node_pair_t;
void t425_node_pair_init(t425_node_pair_t*n,const t425_occam_program_t*a,const t425_occam_program_t*b);
int t425_node_pair_run(t425_node_pair_t*n,uint32_t max_steps);
