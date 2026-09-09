#include <stdint.h>
#include <stddef.h>
#include "t425_flash_layout.h"
#include "t425_flash_slots.h"

_Static_assert(T425_FLASH_CONFIG_REGION_SIZE == 8192u, "config region");
_Static_assert(T425_FLASH_SLOT_SIZE == 4096u, "slot size");
_Static_assert(T425_FLASH_SLOT_SIZE * 2u == T425_FLASH_CONFIG_REGION_SIZE, "A/B");
_Static_assert(T425_FLASH_SLOT_SIZE % 256u == 0u, "page program");
_Static_assert(T425_FLASH_SLOT_SIZE % 4096u == 0u, "sector erase");
_Static_assert(sizeof(t425_flash_slot_t) <= T425_FLASH_SLOT_SIZE,
               "slot record must fit in one 4 KiB sector");
