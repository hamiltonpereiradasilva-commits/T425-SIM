#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "t425_flash_slots.h"

typedef struct {
    bool present;
    t425_config_record_t record;
} t425_config_staging_t;

void t425_config_staging_clear(t425_config_staging_t *s);
bool t425_config_staging_set(t425_config_staging_t *s,
                             const uint8_t *payload, uint16_t len);
bool t425_config_staging_get(const t425_config_staging_t *s,
                             uint8_t *payload, uint16_t cap, uint16_t *len);
bool t425_config_staging_valid(const t425_config_staging_t *s);
