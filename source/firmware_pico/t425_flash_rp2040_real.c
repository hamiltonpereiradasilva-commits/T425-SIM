/*
 * T425-SIM v2.79 — RP2040 physical Flash binding.
 */
#include "t425_flash_rp2040_real.h"
#include "t425_flash_layout.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include <string.h>
#include <stdint.h>

extern char __t425_config_start__;
extern char __t425_config_end__;

static uintptr_t cfg_start(void) { return (uintptr_t)&__t425_config_start__; }
static uintptr_t cfg_end(void)   { return (uintptr_t)&__t425_config_end__; }

static bool region_valid(void) {
    uintptr_t s=cfg_start(), e=cfg_end();
    return e > s &&
           (e-s) == T425_FLASH_CONFIG_REGION_SIZE &&
           (s % FLASH_SECTOR_SIZE) == 0 &&
           (e % FLASH_SECTOR_SIZE) == 0;
}

static uintptr_t slot_addr(uint8_t slot) {
    return cfg_start() + ((uintptr_t)slot * T425_FLASH_SLOT_SIZE);
}

static bool slot_valid(uint8_t slot) {
    if (slot > 1 || !region_valid()) return false;
    uintptr_t s=slot_addr(slot);
    return s >= cfg_start() &&
           s + T425_FLASH_SLOT_SIZE <= cfg_end() &&
           (s % FLASH_SECTOR_SIZE) == 0 &&
           (s % FLASH_PAGE_SIZE) == 0;
}

typedef struct {
    uint32_t offset;
    size_t count;
} flash_erase_args_t;

typedef struct {
    uint32_t offset;
    const uint8_t *data;
    size_t count;
} flash_program_args_t;

static void call_erase(void *param) {
    const flash_erase_args_t *a=(const flash_erase_args_t *)param;
    flash_range_erase(a->offset,a->count);
}

static void call_program(void *param) {
    const flash_program_args_t *a=(const flash_program_args_t *)param;
    flash_range_program(a->offset,a->data,a->count);
}

bool t425_flash_real_read_slot(uint8_t slot, t425_flash_slot_t *out) {
    if (!out || !slot_valid(slot)) return false;
    const uint8_t *xip=(const uint8_t *)slot_addr(slot);
    memcpy(out,xip,sizeof(*out));
    /* A blank/corrupt slot is data, not a bus/read failure. */
    return true;
}

bool t425_flash_real_write_slot(uint8_t slot,
                                const t425_flash_slot_t *in) {
    if (!in || !slot_valid(slot)) return false;
    if (!t425_flash_slot_valid(in)) return false;

    static uint8_t image[T425_FLASH_SLOT_SIZE];
    memset(image,0xFF,sizeof(image));
    memcpy(image,in,sizeof(*in));

    const uint32_t offset=(uint32_t)(slot_addr(slot) - XIP_BASE);

    flash_erase_args_t ea={offset,T425_FLASH_SLOT_SIZE};
    int rc=flash_safe_execute(call_erase,&ea,UINT32_MAX);
    if (rc != PICO_OK) return false;

    flash_program_args_t pa={offset,image,T425_FLASH_SLOT_SIZE};
    rc=flash_safe_execute(call_program,&pa,UINT32_MAX);
    if (rc != PICO_OK) return false;

    t425_flash_slot_t verify;
    const uint8_t *xip=(const uint8_t *)(XIP_BASE + offset);
    memcpy(&verify,xip,sizeof(verify));

    return memcmp(&verify,in,sizeof(verify)) == 0 &&
           t425_flash_slot_valid(&verify);
}


bool t425_flash_real_clear_all(void) {
    if (!region_valid()) return false;

    flash_erase_args_t ea={ (uint32_t)(cfg_start() - XIP_BASE),
                            T425_FLASH_CONFIG_REGION_SIZE };
    return flash_safe_execute(call_erase,&ea,UINT32_MAX) == PICO_OK;
}
