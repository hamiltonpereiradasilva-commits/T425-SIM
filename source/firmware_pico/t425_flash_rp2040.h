#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_flash_slots.h"

typedef struct {
    uint32_t base_offset;
    uint32_t slot_size;
} t425_rp2040_flash_t;

bool t425_rp2040_flash_init(t425_rp2040_flash_t *f,
                            uint32_t slot_a_offset,
                            uint32_t slot_b_offset);
bool t425_rp2040_flash_read(const t425_rp2040_flash_t *f,
                            uint8_t slot, t425_flash_slot_t *out);
bool t425_rp2040_flash_program(const t425_rp2040_flash_t *f,
                               uint8_t slot, const t425_flash_slot_t *in);
