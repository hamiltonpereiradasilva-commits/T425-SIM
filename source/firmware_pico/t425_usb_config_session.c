#include "t425_usb_config_session.h"

void t425_usb_config_session_init(t425_usb_config_session_t *s) {
    if (!s) return;
    t425_config_staging_clear(&s->staging);
    s->commit_pending=false;
}

bool t425_usb_config_session_set(t425_usb_config_session_t *s,
                                 const uint8_t *payload, uint16_t len) {
    if (!s) return false;
    if (!t425_config_staging_set(&s->staging,payload,len)) return false;
    s->commit_pending=false;
    return true;
}

bool t425_usb_config_session_get(const t425_usb_config_session_t *s,
                                 uint8_t *payload, uint16_t cap,
                                 uint16_t *len) {
    return t425_config_staging_get(&s->staging,payload,cap,len);
}
