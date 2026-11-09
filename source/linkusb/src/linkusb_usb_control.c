#include "linkusb_usb_control.h"
#include "tusb.h"
static linkusb_usb_control_t *active_control;

bool linkusb_usb_control_init(linkusb_usb_control_t *control, linkusb_subsystem_t *subsystem) {
    if (!control || !subsystem) return false;
    control->subsystem = subsystem;
    active_control = control;
    return true;
}

bool linkusb_usb_control_apply_mask(linkusb_subsystem_t *subsystem, uint32_t link_id, uint8_t mask) {
    const linkusb_subsystem_signal_t signals[3] = {
        LINKUSB_SUBSYSTEM_RESET, LINKUSB_SUBSYSTEM_ANALYSE, LINKUSB_SUBSYSTEM_ERROR
    };
    const uint8_t bits[3] = {
        LINKUSB_SUBSYSTEM_STATUS_RESET, LINKUSB_SUBSYSTEM_STATUS_ANALYSE, LINKUSB_SUBSYSTEM_STATUS_ERROR
    };
    if (!subsystem || link_id >= LINKUSB_SUBSYSTEM_COUNT || (mask & (uint8_t)~LINKUSB_SUBSYSTEM_SUPPORTED_MASK) != 0u)
        return false;
    for (uint32_t i=0u; i<3u; ++i) {
        bool ok = (mask & bits[i]) != 0u
            ? linkusb_subsystem_assert_signal(subsystem, link_id, signals[i])
            : linkusb_subsystem_release_signal(subsystem, link_id, signals[i]);
        if (!ok) return false;
    }
    return true;
}

void linkusb_usb_control_encode_status(uint8_t link0, uint8_t link1, uint8_t out[2]) {
    if (!out) return;
    out[0] = link0;
    out[1] = link1;
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    static uint8_t status[2];
    (void)rhport;
    if (stage != CONTROL_STAGE_SETUP) return true;
    if (!request || !active_control || !active_control->subsystem) return false;
    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR ||
        request->bmRequestType_bit.recipient != TUSB_REQ_RCPT_DEVICE)
        return false;
    if (request->bRequest == LINKUSB_VENDOR_REQUEST_IDENTIFY || request->bRequest == LINKUSB_VENDOR_REQUEST_BOOT_ERROR)
        return false;
    if (request->bRequest == LINKUSB_VENDOR_REQUEST_SUBSYS_STATUS) {
        if (request->bmRequestType != 0xC0u || request->wValue != 0u || request->wIndex != 0u || request->wLength != 2u)
            return false;
        linkusb_usb_control_encode_status(
            linkusb_subsystem_status(active_control->subsystem, 0u),
            linkusb_subsystem_status(active_control->subsystem, 1u), status);
        return tud_control_xfer(rhport, request, status, 2u);
    }
    if (request->bRequest == LINKUSB_VENDOR_REQUEST_CONTROL) {
        if (request->bmRequestType != 0x40u || request->wIndex >= LINKUSB_SUBSYSTEM_COUNT || request->wLength != 0u)
            return false;
        if (!linkusb_usb_control_apply_mask(active_control->subsystem, request->wIndex, (uint8_t)request->wValue))
            return false;
        return tud_control_status(rhport, request);
    }
    return false;
}
