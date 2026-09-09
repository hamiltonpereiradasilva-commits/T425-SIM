#pragma once
#include <stdint.h>
#include "t425_flash_layout.h"

_Static_assert(T425_FLASH_CONFIG_REGION_SIZE == 8192u,
               "T425 config region must be exactly 8 KiB");
_Static_assert(T425_FLASH_SLOT_SIZE == 4096u,
               "T425 slot must be exactly one Flash sector");
_Static_assert((T425_FLASH_SLOT_SIZE * 2u) ==
               T425_FLASH_CONFIG_REGION_SIZE,
               "A/B slots must exactly fill config region");
_Static_assert((T425_FLASH_SLOT_SIZE % 256u) == 0u,
               "slot must be page-program compatible");
_Static_assert((T425_FLASH_SLOT_SIZE % 4096u) == 0u,
               "slot must be sector-erase compatible");
