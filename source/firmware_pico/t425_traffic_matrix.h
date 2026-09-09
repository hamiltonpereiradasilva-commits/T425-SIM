#pragma once
#include <stdint.h>

typedef struct {
    uint8_t src;
    uint8_t dst;
    uint8_t first_hop;
} t425_traffic_case_t;

static const t425_traffic_case_t T425_V250_CASES[] = {
    {0,3,1}, {3,0,2}, {1,3,2}, {2,0,1},
    {0,2,1}, {3,1,2}, {1,0,0}, {2,3,3}
};

#define T425_V250_CASE_COUNT \
    (sizeof(T425_V250_CASES)/sizeof(T425_V250_CASES[0]))
