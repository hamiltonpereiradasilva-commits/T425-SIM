#include "t425_usb_config_protocol.h"
#include "t425_usb_config_session.h"
#include "t425_usb_config_commit.h"
#include "t425_flash_hw.h"
#include <string.h>

static t425_usb_config_session_t g_session;

static void response(uint8_t status, const uint8_t *payload, uint16_t plen,
                     uint8_t *out, size_t cap, size_t *n) {
    if (!out || !n || cap < (size_t)1 + plen) return;
    out[0]=status;
    if (payload && plen) memcpy(out+1,payload,plen);
    *n=(size_t)1+plen;
}

bool t425_usb_config_handle(const uint8_t *frame, size_t len,
                            uint8_t *out, size_t cap, size_t *out_len) {
    if (!out_len) return false;
    *out_len=0;
    if (!frame || !out || len < sizeof(t425_usb_cfg_header_t)) {
        response(T425_USB_RSP_BAD_FRAME,0,0,out,cap,out_len);
        return false;
    }

    t425_usb_cfg_header_t h;
    memcpy(&h,frame,sizeof(h));

    if (h.magic != T425_USB_CFG_MAGIC ||
        h.version != T425_USB_CFG_VERSION ||
        (size_t)sizeof(h)+h.payload_len != len) {
        response(T425_USB_RSP_BAD_FRAME,0,0,out,cap,out_len);
        return false;
    }

    const uint8_t *payload=frame+sizeof(h);

    switch ((t425_usb_cfg_cmd_t)h.command) {
    case T425_USB_CMD_PING:
        response(T425_USB_RSP_OK,0,0,out,cap,out_len);
        return true;

    case T425_USB_CMD_GET_CONFIG: {
        uint8_t tmp[sizeof(t425_config_record_t)];
        uint16_t n=0;
        if (!t425_usb_config_session_get(&g_session,tmp,sizeof(tmp),&n)) {
            response(T425_USB_RSP_BAD_CONFIG,0,0,out,cap,out_len);
            return false;
        }
        response(T425_USB_RSP_OK,tmp,n,out,cap,out_len);
        return true;
    }

    case T425_USB_CMD_SET_CONFIG:
        if (!t425_usb_config_session_set(&g_session,payload,h.payload_len)) {
            response(T425_USB_RSP_BAD_CONFIG,0,0,out,cap,out_len);
            return false;
        }
        response(T425_USB_RSP_OK,0,0,out,cap,out_len);
        return true;

    case T425_USB_CMD_COMMIT: {
        t425_flash_image_t current, after;
        t425_slot_id_t active;
        if (!t425_flash_hw_boot(&current,&active)) {
            response(T425_USB_RSP_FLASH_FAIL,0,0,out,cap,out_len);
            return false;
        }
        if (!t425_usb_config_commit(&g_session.staging,&current,
                                    &after,&active)) {
            response(T425_USB_RSP_FLASH_FAIL,0,0,out,cap,out_len);
            return false;
        }
        t425_usb_config_session_init(&g_session);
        response(T425_USB_RSP_OK,0,0,out,cap,out_len);
        return true;
    }

    case T425_USB_CMD_REBOOT:
        response(T425_USB_RSP_OK,0,0,out,cap,out_len);
        return true;

    default:
        response(T425_USB_RSP_BAD_COMMAND,0,0,out,cap,out_len);
        return false;
    }
}
