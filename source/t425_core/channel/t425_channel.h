#ifndef T425_CORE_CHANNEL_H
#define T425_CORE_CHANNEL_H

#include <stdint.h>
#include "t425_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * T425 architectural channel control word.
 *
 * Empty channel:
 *
 *     MinInt / NotProcess = 0x80000000
 *
 * Waiting channel:
 *
 *     Wdesc of the process waiting for the rendezvous.
 *
 * The channel control word resides in T425 memory.
 * No FIFO or private rendezvous state is introduced.
 */
#define T425_CHANNEL_EMPTY 0x80000000u

int t425_channel_read_word(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t *value);

int t425_channel_write_word(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t value);

int t425_channel_reset(
    t425_memory_t *memory,
    uint32_t address);

int t425_channel_is_empty(
    uint32_t value);

int t425_channel_wait(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t wdesc);

int t425_channel_take(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t *wdesc);

#ifdef __cplusplus
}
#endif

#endif
