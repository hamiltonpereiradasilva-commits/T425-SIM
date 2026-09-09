#include "t425_usb_config_commit.h"
#include "t425_config_canonical.h"
#include "t425_flash_transaction.h"

bool t425_usb_config_commit(const t425_config_staging_t *staging,
                            const t425_flash_image_t *current,
                            t425_flash_image_t *after,
                            t425_slot_id_t *active_slot) {
    if (!staging || !current || !after || !active_slot ||
        !t425_config_staging_valid(staging))
        return false;

    if (!t425_config_canonical_validate(&staging->record))
        return false;

    return t425_flash_transaction(current,
                                  &staging->record,
                                  after,
                                  active_slot) == T425_FLASH_TX_OK;
}
