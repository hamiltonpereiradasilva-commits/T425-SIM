#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_MAX_ALTS 8

typedef struct {
    uint32_t time;
    bool enabled;
    bool fired;
} t425_timer_guard_t;

typedef struct {
    uint32_t now;
    t425_timer_guard_t guard[T425_MAX_ALTS];
} t425_timer_t;

void t425_timer_init(t425_timer_t*t);
void t425_timer_tick(t425_timer_t*t,uint32_t ticks);
int t425_timer_tin(t425_timer_t*t,uint32_t index,uint32_t deadline);
void t425_timer_enbt(t425_timer_t*t,uint32_t index);
bool t425_timer_ready(t425_timer_t*t,uint32_t index);
