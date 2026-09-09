#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "t425_config_staging.h"
#include "t425_flash_slots.h"

bool t425_usb_config_commit(const t425_config_staging_t *staging,
                            const t425_flash_image_t *current,
                            t425_flash_image_t *after,
                            t425_slot_id_t *active_slot);
