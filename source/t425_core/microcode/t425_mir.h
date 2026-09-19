#ifndef T425_MIR_H
#define T425_MIR_H

#include <stdint.h>

#include "t425_crom.h"

/*
 * IMS T425 microword.
 *
 * The MIR preserves one complete CROM row without interpreting
 * any field.  A microword contains exactly 118 bits.
 */
typedef struct
{
    uint8_t bit[T425_CROM_BITS];
} t425_mir_t;

/*
 * Load one CROM row into the MIR representation.
 *
 * No field decoding is performed.
 * No next-state computation is performed.
 */
void t425_mir_load(
    t425_mir_t *mir,
    uint16_t crom_index
);

#endif /* T425_MIR_H */
