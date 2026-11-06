#ifndef T425_BYTE_WRITE_H
#define T425_BYTE_WRITE_H

#include <stdint.h>

uint8_t t425_byte_write_lane_mask(
    uint32_t areg, uint32_t breg, uint32_t dreg,
    uint8_t mir_strobe_enable, uint8_t strobe_latch);

#endif
