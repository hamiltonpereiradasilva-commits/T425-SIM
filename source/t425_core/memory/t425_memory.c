#include "t425_memory.h"

static int valid_byte_address(uint32_t address)
{
    return address >= T425_INTERNAL_RAM_BASE &&
           address < T425_INTERNAL_RAM_END;
}

static int valid_word_address(uint32_t address)
{
    return valid_byte_address(address) &&
           (address & 3u) == 0u &&
           address <= T425_INTERNAL_RAM_END - 4u;
}

static uint32_t index_of(uint32_t address)
{
    return address - T425_INTERNAL_RAM_BASE;
}

void t425_memory_reset(t425_memory_t *mem)
{
    if (!mem)
        return;

    for (uint32_t i = 0; i < T425_INTERNAL_RAM_BYTES; ++i)
        mem->data[i] = 0;

    (void)t425_memory_write_word(mem, T425_TPTR_LOC0, 0x80000000u);
    (void)t425_memory_write_word(mem, T425_TPTR_LOC1, 0x80000000u);
}

int t425_memory_read_byte(
    const t425_memory_t *mem,
    uint32_t address,
    uint8_t *value)
{
    if (!mem || !value || !valid_byte_address(address))
        return -1;

    *value = mem->data[index_of(address)];
    return 0;
}

int t425_memory_write_byte(
    t425_memory_t *mem,
    uint32_t address,
    uint8_t value)
{
    if (!mem || !valid_byte_address(address))
        return -1;

    mem->data[index_of(address)] = value;
    return 0;
}

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value)
{
    uint32_t i;

    if (!mem || !value || !valid_word_address(address))
        return -1;

    i = index_of(address);

    *value =
        ((uint32_t)mem->data[i + 0u] << 0)  |
        ((uint32_t)mem->data[i + 1u] << 8)  |
        ((uint32_t)mem->data[i + 2u] << 16) |
        ((uint32_t)mem->data[i + 3u] << 24);

    return 0;
}

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value)
{
    uint32_t i;

    if (!mem || !valid_word_address(address))
        return -1;

    i = index_of(address);

    mem->data[i + 0u] = (uint8_t)(value >> 0);
    mem->data[i + 1u] = (uint8_t)(value >> 8);
    mem->data[i + 2u] = (uint8_t)(value >> 16);
    mem->data[i + 3u] = (uint8_t)(value >> 24);

    return 0;
}
