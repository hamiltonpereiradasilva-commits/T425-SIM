#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_flash_slots.h"

bool t425_flash_real_read_slot(uint8_t slot, t425_flash_slot_t *out);
bool t425_flash_real_write_slot(uint8_t slot,
                                const t425_flash_slot_t *in);
bool t425_flash_real_clear_all(void);
