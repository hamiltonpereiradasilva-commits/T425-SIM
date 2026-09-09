#include "t425_flash_transaction.h"
#include "t425_flash_hw.h"
#include <string.h>

t425_flash_tx_status_t
t425_flash_transaction(const t425_flash_image_t *current,
                       const t425_config_record_t *new_record,
                       t425_flash_image_t *after,
                       t425_slot_id_t *new_active) {
    if (!current || !new_record || !after || !new_active)
        return T425_FLASH_TX_BAD_ARGUMENT;
    if (!t425_config_validate(new_record))
        return T425_FLASH_TX_BAD_ARGUMENT;

    t425_slot_id_t target = T425_SLOT_NONE;
    t425_slot_id_t active = t425_flash_select_active(current);

    /*
     * Virgin device: both slots invalid. The first commit goes to A.
     * Otherwise always write the inactive slot.
     */
    if (active == T425_SLOT_A) target = T425_SLOT_B;
    else if (active == T425_SLOT_B) target = T425_SLOT_A;
    else target = T425_SLOT_A;

    if (!t425_flash_prepare_commit(current,new_record,after,&target))
        return T425_FLASH_TX_BAD_ARGUMENT;

    const uint8_t index=(target == T425_SLOT_A) ? 0u : 1u;

    if (!t425_flash_hw_write_slot(index,
        target == T425_SLOT_A ? &after->a : &after->b))
        return T425_FLASH_TX_PROGRAM_FAILED;

    /*
     * Verify the target directly from hardware after programming.
     * A successful transaction is allowed to change only the target slot.
     */
    t425_flash_slot_t rb;
    if (!t425_flash_hw_read_slot(index,&rb))
        return T425_FLASH_TX_VERIFY_FAILED;
    if (!t425_flash_slot_valid(&rb))
        return T425_FLASH_TX_VERIFY_FAILED;

    if (memcmp(&rb,
               target == T425_SLOT_A ? &after->a : &after->b,
               sizeof(rb)) != 0)
        return T425_FLASH_TX_VERIFY_FAILED;

    t425_flash_image_t actual=*current;
    if (target == T425_SLOT_A) actual.a=rb;
    else actual.b=rb;

    if (t425_flash_select_active(&actual) != target)
        return T425_FLASH_TX_VERIFY_FAILED;

    *after=actual;
    *new_active=target;
    return T425_FLASH_TX_OK;
}
