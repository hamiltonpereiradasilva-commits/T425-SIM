#include "t425_memory.h"

static int valid_word(uint32_t address)
{
    return address < T425_INTERNAL_RAM_BYTES &&
           (address & 3u) == 0u;
}

void t425_memory_reset(t425_memory_t *mem)
{
    if (!mem)
        return;

    for (uint32_t i = 0; i < T425_INTERNAL_RAM_BYTES; ++i)
        mem->data[i] = 0;
}

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value)
{
    if (!mem || !value || !valid_word(address))
        return -1;

    *value =
        ((uint32_t)mem->data[address + 0] << 0)  |
        ((uint32_t)mem->data[address + 1] << 8)  |
        ((uint32_t)mem->data[address + 2] << 16) |
        ((uint32_t)mem->data[address + 3] << 24);

    return 0;
}

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value)
{
    if (!mem || !valid_word(address))
        return -1;

    mem->data[address + 0] = (uint8_t)(value >> 0);
    mem->data[address + 1] = (uint8_t)(value >> 8);
    mem->data[address + 2] = (uint8_t)(value >> 16);
    mem->data[address + 3] = (uint8_t)(value >> 24);

    return 0;
}
