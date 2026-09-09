#pragma once
#include <stdint.h>

extern const uint8_t __t425_config_start__;
extern const uint8_t __t425_config_end__;

static inline uintptr_t t425_config_flash_base(void) {
    return (uintptr_t)&__t425_config_start__;
}

static inline uintptr_t t425_config_flash_end(void) {
    return (uintptr_t)&__t425_config_end__;
}
