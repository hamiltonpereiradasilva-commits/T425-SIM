/*
 * V2.62 physical Flash integration contract.
 *
 * This file deliberately contains no erase/program operation until the
 * authoritative linker script exports __t425_config_start__/end.
 */
#include "t425_config_address.h"
#include "t425_flash_layout.h"
#include <stdbool.h>

bool t425_config_region_is_valid(void) {
    uintptr_t start=t425_config_flash_base();
    uintptr_t end=t425_config_flash_end();
    return end > start &&
           (end-start) == T425_FLASH_CONFIG_REGION &&
           (start % T425_FLASH_ERASE_SIZE) == 0;
}
