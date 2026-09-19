#ifndef T425_MICROSEQUENCER_H
#define T425_MICROSEQUENCER_H

#include <stdint.h>

#include "t425_mir.h"

typedef struct
{
    uint16_t mpc;
    uint16_t next_base;
    t425_mir_t mir;
} t425_microsequencer_t;

/*
 * Reset the microsequencer.
 */
void t425_microsequencer_reset(
    t425_microsequencer_t *seq,
    uint16_t initial_mpc);

/*
 * Load MIR from the current MPC.
 *
 * This operation performs:
 *
 *     MPC -> CROM[MPC] -> MIR
 *
 * No conditional next-MPC selection is performed here.
 */
void t425_microsequencer_load_mir(
    t425_microsequencer_t *seq);

/*
 * Extract the unconditional/base next-address field
 * from the currently loaded MIR.
 *
 * This is NOT yet the complete architectural next-MPC
 * selection mechanism.
 */
uint16_t t425_microsequencer_next_base(
    const t425_microsequencer_t *seq);

/*
 * Advance to the base next address.
 *
 * Conditional selectors are intentionally not implemented
 * by this primitive.
 */
void t425_microsequencer_advance_base(
    t425_microsequencer_t *seq);

#endif /* T425_MICROSEQUENCER_H */
