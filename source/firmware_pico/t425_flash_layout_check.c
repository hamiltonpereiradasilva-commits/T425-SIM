#include <stdint.h>
#include "t425_flash_layout.h"

extern char __t425_config_start__;
extern char __t425_config_end__;

_Static_assert(T425_FLASH_CONFIG_REGION_SIZE == 8192u,
               "T425 config region must remain 8 KiB");
_Static_assert((T425_FLASH_CONFIG_REGION_SIZE % 4096u) == 0,
               "config region must be sector aligned");
_Static_assert((T425_FLASH_SLOT_SIZE % 256u) == 0,
               "slot size must be page-program compatible");

void t425_flash_layout_check(void) {
    uintptr_t s=(uintptr_t)&__t425_config_start__;
    uintptr_t e=(uintptr_t)&__t425_config_end__;
    if (e <= s || (e-s) != T425_FLASH_CONFIG_REGION_SIZE) {
        __builtin_trap();
    }
}
