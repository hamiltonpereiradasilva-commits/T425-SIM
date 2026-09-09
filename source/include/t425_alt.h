#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_timer.h"
#include "t425_channels.h"

typedef enum { ALT_NONE=0, ALT_CHANNEL, ALT_TIMER } t425_alt_kind_t;
typedef struct {
    t425_alt_kind_t kind;
    uint16_t channel;
    uint32_t timer_guard;
    bool enabled;
} t425_alt_guard_t;

int t425_alt_select(t425_alt_guard_t *g,uint16_t n,
                    t425_channel_t *channels,t425_timer_t *timer);
