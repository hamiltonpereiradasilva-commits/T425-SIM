#include "t425_flash_rp2040.h"
#include "t425_flash_layout.h"

/*
 * Hardware binding point.
 *
 * Deliberately no SDK-specific erase/program call is issued here until the
 * application's linker map reserves the final region. This file establishes
 * the RP2040-facing contract and validates alignment.
 */

bool t425_rp2040_flash_init(t425_rp2040_flash_t *f,
                            uint32_t slot_a_offset,
                            uint32_t slot_b_offset) {
    if (!f) return false;
    if (slot_b_offset != slot_a_offset + T425_FLASH_SLOT_SIZE) return false;
    if ((slot_a_offset % T425_FLASH_ERASE_SIZE) != 0 ||
        (slot_b_offset % T425_FLASH_ERASE_SIZE) != 0) return false;
    f->base_offset=slot_a_offset;
    f->slot_size=T425_FLASH_SLOT_SIZE;
    return true;
}

bool t425_rp2040_flash_read(const t425_rp2040_flash_t *f,
                            uint8_t slot, t425_flash_slot_t *out) {
    (void)f; (void)slot; (void)out;
    return false; /* bind to XIP flash read after final layout */
}

bool t425_rp2040_flash_program(const t425_rp2040_flash_t *f,
                               uint8_t slot, const t425_flash_slot_t *in) {
    (void)f; (void)slot; (void)in;
    return false; /* bind erase/program after final layout */
}
