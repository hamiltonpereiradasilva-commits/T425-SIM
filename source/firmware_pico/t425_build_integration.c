#include "t425_build_integration.h"
#include "t425_flash_layout.h"
#include <stdbool.h>

bool t425_build_memory_contract_valid(void) {
    return (T425_APP_FLASH_BYTES + T425_CONFIG_BYTES)
            <= T425_FLASH_TOTAL_BYTES;
}
