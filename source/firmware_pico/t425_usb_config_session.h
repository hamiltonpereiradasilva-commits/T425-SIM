#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "t425_config_staging.h"

typedef struct {
    t425_config_staging_t staging;
    bool commit_pending;
} t425_usb_config_session_t;

void t425_usb_config_session_init(t425_usb_config_session_t *s);
bool t425_usb_config_session_set(t425_usb_config_session_t *s,
                                 const uint8_t *payload, uint16_t len);
bool t425_usb_config_session_get(const t425_usb_config_session_t *s,
                                 uint8_t *payload, uint16_t cap,
                                 uint16_t *len);
