#include "tusb.h"
#include "linkusb_usb_bulk.h"
#define LINKUSB_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + (2u * TUD_VENDOR_DESC_LEN))
enum { LINKUSB_ITF0=0, LINKUSB_ITF1=1, LINKUSB_ITF_COUNT=2 };
tusb_desc_device_t const desc_device = {
    .bLength=sizeof(tusb_desc_device_t), .bDescriptorType=TUSB_DESC_DEVICE,
    .bcdUSB=0x0200, .bDeviceClass=0xff, .bDeviceSubClass=0,
    .bDeviceProtocol=0, .bMaxPacketSize0=CFG_TUD_ENDPOINT0_SIZE,
    .idVendor=0x1209, .idProduct=0x4250, .bcdDevice=0x0100,
    .iManufacturer=1, .iProduct=2, .iSerialNumber=3, .bNumConfigurations=1
};
uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, LINKUSB_ITF_COUNT, 0, LINKUSB_CONFIG_TOTAL_LEN, 0x80, 100),
    TUD_VENDOR_DESCRIPTOR(LINKUSB_ITF0, 4, LINKUSB_EP_LINK0_OUT, LINKUSB_EP_LINK0_IN, USB_LINK_BULK_PACKET_SIZE),
    TUD_VENDOR_DESCRIPTOR(LINKUSB_ITF1, 5, LINKUSB_EP_LINK1_OUT, LINKUSB_EP_LINK1_IN, USB_LINK_BULK_PACKET_SIZE)
};
uint8_t const * tud_descriptor_device_cb(void) { return (uint8_t const *)&desc_device; }
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) { (void)index; return desc_configuration; }
uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t desc_str[32];
    static const char *names[]={"","Transputer Revival","LINKUSB Bulk","Link0 Bulk","Link1 Bulk"};
    if (index>=sizeof(names)/sizeof(names[0])) return 0;
    uint8_t len=0u; while(names[index][len] && len<31u){desc_str[1u+len]=names[index][len];++len;}
    desc_str[0]=(uint16_t)((TUSB_DESC_STRING<<8)|(2u*len+2u));
    if(index==0u) desc_str[1]=0x0409u, desc_str[0]=0x0304u;
    return desc_str;
}
