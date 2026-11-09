#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "linkusb.h"

static linkusb_adapter_t linkusb;

int main(void)
{
    stdio_init_all();

    /*
     * USB-LINK-03B diagnostic gate only.
     *
     * CDC is used exclusively to make the one-shot bring-up markers
     * observable by the host. It is NOT the LINKUSB data plane.
     * The LINKUSB data plane will use USB Bulk endpoints.
     */
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }

    sleep_ms(250);

    printf("\n");
    printf("===== USB-LINK-03B =====\n");
    printf("LINKUSB_PHYSICAL_LINK_COUNT=2\n");
    printf("LINKUSB_PIO=PIO0\n");
    printf("LINK0=SM0_TX/SM1_RX/GPIO0/GPIO1\n");
    printf("LINK1=SM2_TX/SM3_RX/GPIO19/GPIO18\n");

    if (!linkusb_adapter_init(&linkusb)) {
        printf("LINKUSB_ADAPTER_INIT=FAIL\n");
        while (true)
            tight_loop_contents();
    }

    printf("LINKUSB_ADAPTER_INIT=PASS\n");

    if (!linkusb_adapter_start(&linkusb)) {
        printf("LINKUSB_ADAPTER_START=FAIL\n");
        linkusb_adapter_deinit(&linkusb);

        while (true)
            tight_loop_contents();
    }

    printf("LINKUSB_ADAPTER_START=PASS\n");
    printf("LINKUSB_SHARED_PIO_PROGRAMS=PASS\n");
    printf("LINKUSB_CORE_INSTANTIATED=NO\n");
    printf("LINKUSB_USB_BULK_IMPLEMENTED=NO\n");
    printf("===== USB-LINK-03B PASS =====\n");

    while (true) {
        linkusb_adapter_service(&linkusb);
        tight_loop_contents();
    }
}
