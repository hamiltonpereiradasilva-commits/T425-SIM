#ifndef T425_PREVREAD_FORWARDING_87_H
#define T425_PREVREAD_FORWARDING_87_H

#include <stdint.h>

/*
 * Experimental model of the EMUL64 DataOutfromByteUpShifterOutput gates.
 * The latch argument is the live ByteWriteStrobeEnableLatch value at the
 * datapath call, after any current MIR strobe SET has been applied.
 */
static inline int t425_core_f87_prevread_write_enabled(
    uint8_t data_reg_from_byte_shifter,
    uint8_t byte_write_strobe_enable,
    uint8_t byte_write_strobe_enable_latch_live,
    uint8_t mem_write)
{
    if (data_reg_from_byte_shifter == 0u)
        return 0;
    if (byte_write_strobe_enable_latch_live != 0u)
        return 1;
    return byte_write_strobe_enable != 0u && mem_write != 0u;
}

static inline uint32_t t425_core_f87_prevread_next(
    uint32_t previous_read_pre,
    uint32_t data_in_pre,
    uint8_t data_reg_from_byte_shifter,
    uint8_t byte_write_strobe_enable,
    uint8_t byte_write_strobe_enable_latch_live,
    uint8_t mem_write)
{
    return t425_core_f87_prevread_write_enabled(
               data_reg_from_byte_shifter,
               byte_write_strobe_enable,
               byte_write_strobe_enable_latch_live,
               mem_write) != 0 ? data_in_pre : previous_read_pre;
}

#endif
