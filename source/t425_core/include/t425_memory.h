#ifndef T425_MEMORY_H
#define T425_MEMORY_H

#include <stdint.h>

#define T425_INTERNAL_RAM_BASE 0x80000000u
#define T425_INTERNAL_RAM_BYTES 4096u
#define T425_INTERNAL_RAM_END \
    (T425_INTERNAL_RAM_BASE + T425_INTERNAL_RAM_BYTES)

#define T425_TPTR_LOC0 0x80000024u
#define T425_TPTR_LOC1 0x80000028u


typedef struct
{
    uint8_t data[T425_INTERNAL_RAM_BYTES];
} t425_memory_t;

void t425_memory_reset(t425_memory_t *mem);

int t425_memory_read_byte(
    const t425_memory_t *mem,
    uint32_t address,
    uint8_t *value);

int t425_memory_write_byte(
    t425_memory_t *mem,
    uint32_t address,
    uint8_t value);

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value);

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value);

#endif
