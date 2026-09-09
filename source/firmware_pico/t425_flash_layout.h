#pragma once
#include <stdint.h>

/*
 * Flash layout is expressed relative to the end of the application image.
 * The final linker/application integration must reserve this region.
 */
#define T425_FLASH_SLOT_SIZE      4096u
#define T425_FLASH_CONFIG_REGION  (2u * T425_FLASH_SLOT_SIZE)
#define T425_FLASH_ERASE_SIZE     4096u
#define T425_FLASH_PROGRAM_SIZE   256u
#define T425_FLASH_SLOT_A_INDEX   0u
#define T425_FLASH_SLOT_B_INDEX   1u

#include <stdbool.h>

typedef struct {
    uint32_t slot_a_offset;
    uint32_t slot_b_offset;
    uint32_t slot_size;
} t425_flash_layout_t;

bool t425_flash_layout_validate(const t425_flash_layout_t *layout);

#define T425_FLASH_CONFIG_REGION_SIZE 8192u

#define T425_FLASH_PAGE_SIZE 256u
