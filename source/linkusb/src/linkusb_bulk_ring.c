#include "linkusb_bulk_ring.h"
void linkusb_bulk_ring_init(linkusb_bulk_ring_t *ring) {
    if (ring) { ring->head=0u; ring->tail=0u; ring->count=0u; }
}
bool linkusb_bulk_ring_push(linkusb_bulk_ring_t *ring, uint8_t value) {
    if (!ring || ring->count >= USB_LINK_HOST_TX_BUFFER_SIZE) return false;
    ring->data[ring->head] = value;
    ring->head = (uint16_t)((ring->head + 1u) % USB_LINK_HOST_TX_BUFFER_SIZE);
    ++ring->count;
    return true;
}
bool linkusb_bulk_ring_push_front(linkusb_bulk_ring_t *ring, uint8_t value) {
    if (!ring || ring->count >= USB_LINK_HOST_TX_BUFFER_SIZE) return false;
    ring->tail = (uint16_t)((ring->tail + USB_LINK_HOST_TX_BUFFER_SIZE - 1u) % USB_LINK_HOST_TX_BUFFER_SIZE);
    ring->data[ring->tail] = value;
    ++ring->count;
    return true;
}
bool linkusb_bulk_ring_requeue_front(linkusb_bulk_ring_t *ring, const uint8_t *data, uint16_t count) {
    if (!ring || (!data && count != 0u) || count > linkusb_bulk_ring_free(ring)) return false;
    for (uint16_t i=count; i>0u; --i)
        if (!linkusb_bulk_ring_push_front(ring, data[i-1u])) return false;
    return true;
}
bool linkusb_bulk_ring_pop(linkusb_bulk_ring_t *ring, uint8_t *value) {
    if (!ring || !value || ring->count == 0u) return false;
    *value = ring->data[ring->tail];
    ring->tail = (uint16_t)((ring->tail + 1u) % USB_LINK_HOST_TX_BUFFER_SIZE);
    --ring->count;
    return true;
}
uint16_t linkusb_bulk_ring_count(const linkusb_bulk_ring_t *ring) { return ring ? ring->count : 0u; }
uint16_t linkusb_bulk_ring_free(const linkusb_bulk_ring_t *ring) { return ring ? (uint16_t)(USB_LINK_HOST_TX_BUFFER_SIZE - ring->count) : 0u; }
