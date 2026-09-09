#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_flash_slots.h"

bool t425_flash_hw_read_slot(uint8_t slot, t425_flash_slot_t *out);
bool t425_flash_hw_write_slot(uint8_t slot, const t425_flash_slot_t *in);
bool t425_flash_hw_boot(t425_flash_image_t *image,
                        t425_slot_id_t *active_slot);
bool t425_flash_hw_commit(const t425_flash_image_t *current,
                          const t425_config_record_t *new_record,
                          t425_flash_image_t *after,
                          t425_slot_id_t *active_slot);
bool t425_flash_hw_clear_all(void);
