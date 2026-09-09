#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_runtime.h"
#include "t425_scheduler.h"
#include "t425_vm.h"

typedef struct {
    t425_runtime_t runtime;
    t425_scheduler_t scheduler;
    uint32_t boot_count;
    uint32_t diagnostic_commands;
} t425_supervisor_t;

bool t425_supervisor_init(t425_supervisor_t *s,
                          const t425_node_config_t *cfg);
bool t425_supervisor_start(t425_supervisor_t *s);
bool t425_supervisor_step(t425_supervisor_t *s);
bool t425_supervisor_status(const t425_supervisor_t *s,
                            char *buf, uint32_t capacity);
