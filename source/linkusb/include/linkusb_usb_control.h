#ifndef LINKUSB_USB_CONTROL_H
#define LINKUSB_USB_CONTROL_H
#include <stdbool.h>
#include <stdint.h>
#include "linkusb_subsystem.h"
#define LINKUSB_VENDOR_REQUEST_IDENTIFY 0xB0u
#define LINKUSB_VENDOR_REQUEST_BOOT_ERROR 0xB1u
#define LINKUSB_VENDOR_REQUEST_SUBSYS_STATUS 0xB2u
#define LINKUSB_VENDOR_REQUEST_CONTROL 0xB3u
#define LINKUSB_SUBSYSTEM_SUPPORTED_MASK (LINKUSB_SUBSYSTEM_STATUS_RESET | LINKUSB_SUBSYSTEM_STATUS_ANALYSE | LINKUSB_SUBSYSTEM_STATUS_ERROR)
typedef struct { linkusb_subsystem_t *subsystem; } linkusb_usb_control_t;
bool linkusb_usb_control_init(linkusb_usb_control_t *control, linkusb_subsystem_t *subsystem);
bool linkusb_usb_control_apply_mask(linkusb_subsystem_t *subsystem, uint32_t link_id, uint8_t mask);
void linkusb_usb_control_encode_status(uint8_t link0, uint8_t link1, uint8_t out[2]);
#endif
