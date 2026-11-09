#ifndef LINKUSB_USB_BULK_H
#define LINKUSB_USB_BULK_H
#include <stdbool.h>
#include <stdint.h>
#include "linkusb_adapter.h"
#include "linkusb_bulk_ring.h"
#define LINKUSB_USB_INTERFACE_COUNT 2u
typedef struct {
    linkusb_adapter_t *adapter;
    linkusb_bulk_ring_t host_tx[LINKUSB_PHYSICAL_LINK_COUNT];
    linkusb_bulk_ring_t host_rx[LINKUSB_PHYSICAL_LINK_COUNT];
} linkusb_usb_bulk_t;
bool linkusb_usb_bulk_init(linkusb_usb_bulk_t *bulk, linkusb_adapter_t *adapter);
void linkusb_usb_service(linkusb_usb_bulk_t *bulk);
bool linkusb_usb_host_tx_push(linkusb_usb_bulk_t *bulk, uint32_t link_id, uint8_t data);
bool linkusb_usb_host_rx_pop(linkusb_usb_bulk_t *bulk, uint32_t link_id, uint8_t *data);
uint16_t linkusb_usb_host_tx_pending(const linkusb_usb_bulk_t *bulk, uint32_t link_id);
uint16_t linkusb_usb_host_rx_pending(const linkusb_usb_bulk_t *bulk, uint32_t link_id);
#endif
