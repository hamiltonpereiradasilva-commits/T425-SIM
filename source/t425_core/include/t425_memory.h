#ifndef T425_MEMORY_H
#define T425_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define T425_INTERNAL_RAM_BASE 0x80000000u
#define T425_INTERNAL_RAM_BYTES 4096u
#define T425_INTERNAL_RAM_END \
    (T425_INTERNAL_RAM_BASE + T425_INTERNAL_RAM_BYTES)

#define T425_TPTR_LOC0 0x80000024u
#define T425_TPTR_LOC1 0x80000028u
#define T425_MEM_START 0x80000070u
#define T425_ROM_ENTRY 0x7FFFFFFEu

typedef int (*t425_memory_external_read_fn)(
    void *context, uint32_t address, uint8_t *data, size_t size);
typedef int (*t425_memory_external_write_fn)(
    void *context, uint32_t address, const uint8_t *data, size_t size);
/* Return 0 after an atomic commit; nonzero guarantees no lanes changed. */
typedef int (*t425_memory_external_masked_write_fn)(
    void *context, uint32_t aligned_address, uint32_t value,
    uint32_t byte_mask);

typedef struct
{
    uint8_t data[T425_INTERNAL_RAM_BYTES];
    t425_memory_external_read_fn external_read;
    t425_memory_external_write_fn external_write;
    t425_memory_external_masked_write_fn external_masked_write;
    void *external_context;
} t425_memory_t;

void t425_memory_reset(t425_memory_t *mem);

void t425_memory_set_external_backend(
    t425_memory_t *mem,
    t425_memory_external_read_fn external_read,
    t425_memory_external_write_fn external_write,
    void *context);

void t425_memory_set_external_masked_write_backend(
    t425_memory_t *mem,
    t425_memory_external_masked_write_fn external_masked_write);

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

/*
 * Synchronous functional-model masked write. Internal RAM updates selected
 * byte lanes in one call. External masked writes require the atomic masked
 * backend; this function never emulates them with read-modify-write. Callers
 * must serialize CPU and channel-DMA memory operations.
 */
int t425_memory_write_masked_word(
    t425_memory_t *mem,
    uint32_t aligned_address,
    uint32_t value,
    uint32_t byte_mask);

#endif
