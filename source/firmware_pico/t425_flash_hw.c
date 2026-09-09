#include "t425_flash_hw.h"
#include "t425_flash_rp2040_real.h"
#include "t425_flash_transaction.h"

/*
 * Hardware facade.
 * The transaction layer never calls RP2040 primitives directly.
 */
bool t425_flash_hw_read_slot(uint8_t slot, t425_flash_slot_t *out) {
    return t425_flash_real_read_slot(slot,out);
}

bool t425_flash_hw_write_slot(uint8_t slot, const t425_flash_slot_t *in) {
    return t425_flash_real_write_slot(slot,in);
}

bool t425_flash_hw_boot(t425_flash_image_t *image,
                        t425_slot_id_t *active_slot) {
    if (!image || !active_slot) return false;
    if (!t425_flash_hw_read_slot(0,&image->a)) return false;
    if (!t425_flash_hw_read_slot(1,&image->b)) return false;
    *active_slot=t425_flash_select_active(image);
    return true;
}

bool t425_flash_hw_commit(const t425_flash_image_t *current,
                          const t425_config_record_t *new_record,
                          t425_flash_image_t *after,
                          t425_slot_id_t *active_slot) {
    return t425_flash_transaction(current,new_record,after,active_slot)
           == T425_FLASH_TX_OK;
}


bool t425_flash_hw_clear_all(void) {
    return t425_flash_real_clear_all();
}
