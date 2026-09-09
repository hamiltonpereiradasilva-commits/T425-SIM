#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "t425_flash_slots.h"

typedef enum {
    T425_FLASH_TX_OK = 0,
    T425_FLASH_TX_BAD_ARGUMENT,
    T425_FLASH_TX_NO_VALID_SLOT,
    T425_FLASH_TX_ERASE_FAILED,
    T425_FLASH_TX_PROGRAM_FAILED,
    T425_FLASH_TX_VERIFY_FAILED,
    T425_FLASH_TX_CRC_FAILED
} t425_flash_tx_status_t;

t425_flash_tx_status_t
t425_flash_transaction(const t425_flash_image_t *current,
                       const t425_config_record_t *new_record,
                       t425_flash_image_t *after,
                       t425_slot_id_t *new_active);
