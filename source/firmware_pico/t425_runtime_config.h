#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "t425_flash_slots.h"

typedef enum {
    T425_BOOT_RECOVERY = 0,
    T425_BOOT_CONFIGURED = 1
} t425_boot_state_t;

typedef struct {
    t425_boot_state_t state;
    t425_config_record_t config;
    uint8_t active_slot;
} t425_runtime_config_t;

bool t425_runtime_config_load(t425_runtime_config_t *rt);
bool t425_runtime_config_validate(const t425_runtime_config_t *rt);
