#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_config_store.h"

typedef struct {
    t425_config_record_t record;
    uint32_t generation;
} t425_flash_slot_t;

typedef enum {
    T425_SLOT_NONE = 0,
    T425_SLOT_A = 1,
    T425_SLOT_B = 2
} t425_slot_id_t;

typedef struct {
    t425_flash_slot_t a;
    t425_flash_slot_t b;
} t425_flash_image_t;

bool t425_flash_slot_valid(const t425_flash_slot_t *s);
t425_slot_id_t t425_flash_select_active(const t425_flash_image_t *img);
bool t425_flash_prepare_commit(const t425_flash_image_t *current,
                               const t425_config_record_t *new_record,
                               t425_flash_image_t *next,
                               t425_slot_id_t *target_slot);
