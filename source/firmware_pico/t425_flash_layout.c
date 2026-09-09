#include "t425_flash_layout.h"

bool t425_flash_layout_validate(const t425_flash_layout_t *l) {
    if (!l || l->slot_size != T425_FLASH_SLOT_SIZE) return false;
    if ((l->slot_a_offset % T425_FLASH_ERASE_SIZE) != 0) return false;
    if ((l->slot_b_offset % T425_FLASH_ERASE_SIZE) != 0) return false;
    if (l->slot_b_offset != l->slot_a_offset + l->slot_size) return false;
    return true;
}
