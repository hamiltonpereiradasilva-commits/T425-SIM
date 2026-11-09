#include "pico/stdlib.h"
#include "tusb.h"
#include "linkusb_usb_bulk.h"
static linkusb_adapter_t adapter;
static linkusb_usb_bulk_t bulk;
int main(void) {
    if (!linkusb_adapter_init(&adapter)) { while (true) tight_loop_contents(); }
    if (!linkusb_adapter_start(&adapter)) { linkusb_adapter_deinit(&adapter); while (true) tight_loop_contents(); }
    (void)linkusb_usb_bulk_init(&bulk,&adapter);
    tusb_init(0);
    while (true) {
        tud_task();
        linkusb_usb_service(&bulk);
        tight_loop_contents();
    }
}
