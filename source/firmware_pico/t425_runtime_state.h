#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "t425_runtime_config.h"

typedef enum {
    T425_RUNTIME_RECOVERY = 0,
    T425_RUNTIME_INITIALIZING,
    T425_RUNTIME_ONLINE
} t425_runtime_state_t;

typedef struct {
    t425_runtime_state_t state;
    uint8_t node_id;
    uint8_t active_slot;
} t425_runtime_state_info_t;

bool t425_runtime_init(t425_runtime_state_info_t *info);
bool t425_runtime_interfaces_ready(void);
bool t425_runtime_router_ready(void);
