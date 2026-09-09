#ifndef T425_CORE_CHANNEL_H
#define T425_CORE_CHANNEL_H

#include <stdint.h>

typedef struct
{
    uint32_t address;

    uint8_t waiting_input;
    uint8_t waiting_output;
    uint8_t enabled;
} t425_channel_state_t;

#endif
