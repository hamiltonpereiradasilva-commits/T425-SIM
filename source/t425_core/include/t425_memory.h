#ifndef T425_MEMORY_H
#define T425_MEMORY_H

#include <stdint.h>

#define T425_INTERNAL_RAM_BYTES 4096u

typedef struct
{
    uint8_t data[T425_INTERNAL_RAM_BYTES];
} t425_memory_t;

void t425_memory_reset(t425_memory_t *mem);

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value);

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value);

#endif
