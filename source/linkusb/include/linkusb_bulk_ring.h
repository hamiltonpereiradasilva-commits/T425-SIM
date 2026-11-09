#ifndef LINKUSB_BULK_RING_H
#define LINKUSB_BULK_RING_H
#include <stdbool.h>
#include <stdint.h>
#define USB_LINK_BULK_PACKET_SIZE 64u
#define USB_LINK_HOST_TX_BUFFER_SIZE 256u
#define USB_LINK_HOST_RX_BUFFER_SIZE 256u
#define LINKUSB_EP_LINK0_OUT 0x02u
#define LINKUSB_EP_LINK1_OUT 0x04u
#define LINKUSB_EP_LINK0_IN  0x86u
#define LINKUSB_EP_LINK1_IN  0x88u
#define LINKUSB_USB_SERVICE_BUDGET 8u
typedef struct {
    uint8_t data[USB_LINK_HOST_TX_BUFFER_SIZE];
    uint16_t head, tail, count;
} linkusb_bulk_ring_t;
void linkusb_bulk_ring_init(linkusb_bulk_ring_t *ring);
bool linkusb_bulk_ring_push(linkusb_bulk_ring_t *ring, uint8_t value);
bool linkusb_bulk_ring_push_front(linkusb_bulk_ring_t *ring, uint8_t value);
bool linkusb_bulk_ring_requeue_front(linkusb_bulk_ring_t *ring, const uint8_t *data, uint16_t count);
bool linkusb_bulk_ring_pop(linkusb_bulk_ring_t *ring, uint8_t *value);
uint16_t linkusb_bulk_ring_count(const linkusb_bulk_ring_t *ring);
uint16_t linkusb_bulk_ring_free(const linkusb_bulk_ring_t *ring);
#endif
