#include "linkusb_usb_bulk.h"
#include "tusb.h"
static uint8_t in_packet[LINKUSB_PHYSICAL_LINK_COUNT][USB_LINK_BULK_PACKET_SIZE];


bool linkusb_usb_bulk_init(linkusb_usb_bulk_t *bulk, linkusb_adapter_t *adapter) {
    if (!bulk || !adapter) return false;
    bulk->adapter = adapter;
    for (uint32_t i=0u; i<LINKUSB_PHYSICAL_LINK_COUNT; ++i) {
        linkusb_bulk_ring_init(&bulk->host_tx[i]);
        linkusb_bulk_ring_init(&bulk->host_rx[i]);
    }
    return true;
}
bool linkusb_usb_host_tx_push(linkusb_usb_bulk_t *bulk, uint32_t id, uint8_t data) {
    return bulk && id < LINKUSB_PHYSICAL_LINK_COUNT && linkusb_bulk_ring_push(&bulk->host_tx[id],data);
}
bool linkusb_usb_host_rx_pop(linkusb_usb_bulk_t *bulk, uint32_t id, uint8_t *data) {
    return bulk && id < LINKUSB_PHYSICAL_LINK_COUNT && linkusb_bulk_ring_pop(&bulk->host_rx[id],data);
}
uint16_t linkusb_usb_host_tx_pending(const linkusb_usb_bulk_t *bulk, uint32_t id) {
    return bulk && id < LINKUSB_PHYSICAL_LINK_COUNT ? linkusb_bulk_ring_count(&bulk->host_tx[id]) : 0u;
}
uint16_t linkusb_usb_host_rx_pending(const linkusb_usb_bulk_t *bulk, uint32_t id) {
    return bulk && id < LINKUSB_PHYSICAL_LINK_COUNT ? linkusb_bulk_ring_count(&bulk->host_rx[id]) : 0u;
}
static void drain_usb_out(linkusb_usb_bulk_t *bulk, uint32_t id) {
    uint8_t buf[USB_LINK_BULK_PACKET_SIZE];
    uint32_t available = tud_vendor_n_available((uint8_t)id);
    uint16_t room = linkusb_bulk_ring_free(&bulk->host_tx[id]);
    uint32_t take = available < room ? available : room;
    if (take > sizeof(buf)) take = sizeof(buf);
    if (take) { uint32_t got = tud_vendor_n_read((uint8_t)id, buf, take); take = got;
        for (uint32_t i=0u; i<take; ++i) (void)linkusb_bulk_ring_push(&bulk->host_tx[id],buf[i]);
    }
}
static void move_host_to_link(linkusb_usb_bulk_t *bulk, uint32_t id) {
    uint8_t value;
    if (linkusb_bulk_ring_count(&bulk->host_tx[id]) &&
        linkusb_adapter_tx_ready(bulk->adapter,id) &&
        linkusb_bulk_ring_pop(&bulk->host_tx[id],&value)) {
        if (!linkusb_adapter_tx_put(bulk->adapter,id,value))
            (void)linkusb_bulk_ring_push(&bulk->host_tx[id],value);
    }
}
static void move_link_to_host(linkusb_usb_bulk_t *bulk, uint32_t id) {
    uint8_t value;
    if (linkusb_bulk_ring_free(&bulk->host_rx[id]) &&
        linkusb_adapter_rx_available(bulk->adapter,id) &&
        linkusb_adapter_rx_get(bulk->adapter,id,&value))
        (void)linkusb_bulk_ring_push(&bulk->host_rx[id],value);
    if (linkusb_bulk_ring_count(&bulk->host_rx[id]) &&
        tud_vendor_n_write_available((uint8_t)id)) {
        uint32_t n = linkusb_bulk_ring_count(&bulk->host_rx[id]);
        if (n > USB_LINK_BULK_PACKET_SIZE) n = USB_LINK_BULK_PACKET_SIZE;
        uint32_t wrote=0u;
        while (wrote<n) {
            if (!linkusb_bulk_ring_pop(&bulk->host_rx[id],&value)) break;
            in_packet[id][wrote++] = value;
        }
        if (wrote) {
            uint32_t accepted=tud_vendor_n_write((uint8_t)id,in_packet[id],wrote);
            if (accepted < wrote) {
                (void)linkusb_bulk_ring_requeue_front(&bulk->host_rx[id], &in_packet[id][accepted], (uint16_t)(wrote - accepted));
            }
            if (accepted) (void)tud_vendor_n_write_flush((uint8_t)id);
        }
    }
}
void linkusb_usb_service(linkusb_usb_bulk_t *bulk) {
    if (!bulk || !bulk->adapter) return;
    for (uint32_t budget=0u; budget<LINKUSB_USB_SERVICE_BUDGET; ++budget) {
        uint32_t id=(bulk->adapter->service_cursor + budget) % LINKUSB_PHYSICAL_LINK_COUNT;
        drain_usb_out(bulk,id);
        move_host_to_link(bulk,id);
        linkusb_adapter_service(bulk->adapter);
        move_link_to_host(bulk,id);
    }
}
