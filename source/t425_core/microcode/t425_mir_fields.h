#ifndef T425_MIR_FIELDS_H
#define T425_MIR_FIELDS_H

#include <stdint.h>

#include "t425_mir.h"

/*
 * Formal field extraction from the 118-bit T425 MIR.
 *
 * Field positions are taken directly from the audited
 * Microword FIELD declarations.
 *
 * No semantic interpretation is performed here.
 * No next-state logic is performed here.
 */

uint16_t t425_mir_xbus(const t425_mir_t *mir);
uint16_t t425_mir_ybus(const t425_mir_t *mir);
uint16_t t425_mir_alu(const t425_mir_t *mir);
uint32_t t425_mir_zbus(const t425_mir_t *mir);
uint8_t  t425_mir_stack(const t425_mir_t *mir);
uint8_t  t425_mir_shift(const t425_mir_t *mir);
uint8_t  t425_mir_cond0(const t425_mir_t *mir);
uint8_t  t425_mir_cond1(const t425_mir_t *mir);
uint8_t  t425_mir_timer(const t425_mir_t *mir);
uint8_t  t425_mir_incrementor(const t425_mir_t *mir);
uint8_t  t425_mir_interface(const t425_mir_t *mir);
uint8_t  t425_mir_byte(const t425_mir_t *mir);
uint8_t  t425_mir_error(const t425_mir_t *mir);
uint16_t t425_mir_channel(const t425_mir_t *mir);
uint16_t t425_mir_next_field(const t425_mir_t *mir);
uint16_t t425_mir_const(const t425_mir_t *mir);

#endif /* T425_MIR_FIELDS_H */
