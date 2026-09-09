#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "t425_flash_slots.h"

#define T425_USB_CFG_MAGIC 0x54343235u /* "T425" */
#define T425_USB_CFG_VERSION 1u

typedef enum {
    T425_USB_CMD_GET_CONFIG = 0x02,
    T425_USB_CMD_SET_CONFIG = 0x03,
    T425_USB_CMD_COMMIT     = 0x04,
    T425_USB_CMD_REBOOT     = 0x05,
    T425_USB_CMD_PING       = 0x01
} t425_usb_cfg_cmd_t;

typedef enum {
    T425_USB_RSP_OK          = 0x00,
    T425_USB_RSP_BAD_FRAME   = 0x01,
    T425_USB_RSP_BAD_COMMAND = 0x02,
    T425_USB_RSP_BAD_CONFIG  = 0x03,
    T425_USB_RSP_FLASH_FAIL  = 0x04,
    T425_USB_RSP_RECOVERY    = 0x05
} t425_usb_cfg_status_t;

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t command;
    uint16_t payload_len;
} __attribute__((packed)) t425_usb_cfg_header_t;

bool t425_usb_config_handle(const uint8_t *frame, size_t len,
                            uint8_t *response, size_t response_cap,
                            size_t *response_len);
