#ifndef T425_MICROENTRY_H
#define T425_MICROENTRY_H

#include <stdint.h>

uint16_t t425_microentry_from_instruction(
    uint8_t opcode,
    uint32_t oreg,
    uint16_t signal);

#endif
