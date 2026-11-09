#include "t425_memory.h"

static int valid_byte_address(uint32_t address)
{
    return address >= T425_INTERNAL_RAM_BASE &&
           address < T425_INTERNAL_RAM_END;
}

static int valid_internal_word_address(uint32_t address)
{
    return valid_byte_address(address) &&
           (address & 3u) == 0u &&
           address <= T425_INTERNAL_RAM_END - 4u;
}

static int valid_external_word_address(uint32_t address)
{
    return !valid_byte_address(address) &&
           (address & 3u) == 0u &&
           address <= UINT32_MAX - 3u;
}

static uint32_t index_of(uint32_t address)
{
    return address - T425_INTERNAL_RAM_BASE;
}

static int external_read(
    const t425_memory_t *mem, uint32_t address, uint8_t *data, size_t size)
{
    if (!mem->external_read)
        return -1;
    return mem->external_read(
        mem->external_context, address, data, size);
}

static int external_write(
    t425_memory_t *mem, uint32_t address, const uint8_t *data, size_t size)
{
    if (!mem->external_write)
        return -1;
    return mem->external_write(
        mem->external_context, address, data, size);
}

void t425_memory_reset(t425_memory_t *mem)
{
    if (!mem)
        return;

    mem->external_read = NULL;
    mem->external_write = NULL;
    mem->external_masked_write = NULL;
    mem->external_context = NULL;

    for (uint32_t i = 0; i < T425_INTERNAL_RAM_BYTES; ++i)
        mem->data[i] = 0;

    (void)t425_memory_write_word(mem, T425_TPTR_LOC0, 0x80000000u);
    (void)t425_memory_write_word(mem, T425_TPTR_LOC1, 0x80000000u);
}

void t425_memory_set_external_backend(
    t425_memory_t *mem,
    t425_memory_external_read_fn read_callback,
    t425_memory_external_write_fn write_callback,
    void *context)
{
    if (!mem)
        return;

    mem->external_read = read_callback;
    mem->external_write = write_callback;
    mem->external_masked_write = NULL;
    mem->external_context = context;
}

void t425_memory_set_external_masked_write_backend(
    t425_memory_t *mem,
    t425_memory_external_masked_write_fn masked_write_callback)
{
    if (!mem)
        return;

    mem->external_masked_write = masked_write_callback;
}

int t425_memory_read_byte(
    const t425_memory_t *mem,
    uint32_t address,
    uint8_t *value)
{
    if (!mem || !value)
        return -1;

    if (valid_byte_address(address)) {
        *value = mem->data[index_of(address)];
        return 0;
    }

    return external_read(mem, address, value, 1u);
}

int t425_memory_write_byte(
    t425_memory_t *mem,
    uint32_t address,
    uint8_t value)
{
    if (!mem)
        return -1;

    if (valid_byte_address(address)) {
        mem->data[index_of(address)] = value;
        return 0;
    }

    return external_write(mem, address, &value, 1u);
}

int t425_memory_read_word(
    const t425_memory_t *mem,
    uint32_t address,
    uint32_t *value)
{
    uint8_t bytes[4];

    if (!mem || !value)
        return -1;

    if (valid_internal_word_address(address)) {
        uint32_t i = index_of(address);
        *value =
            ((uint32_t)mem->data[i + 0u] << 0)  |
            ((uint32_t)mem->data[i + 1u] << 8) |
            ((uint32_t)mem->data[i + 2u] << 16) |
            ((uint32_t)mem->data[i + 3u] << 24);
        return 0;
    }

    if (!valid_external_word_address(address) ||
        external_read(mem, address, bytes, sizeof bytes) != 0)
        return -1;

    *value =
        ((uint32_t)bytes[0] << 0)  |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24);
    return 0;
}

int t425_memory_write_word(
    t425_memory_t *mem,
    uint32_t address,
    uint32_t value)
{
    uint8_t bytes[4] = {
        (uint8_t)(value >> 0),
        (uint8_t)(value >> 8),
        (uint8_t)(value >> 16),
        (uint8_t)(value >> 24)
    };

    if (!mem)
        return -1;

    if (valid_internal_word_address(address)) {
        uint32_t i = index_of(address);
        mem->data[i + 0u] = bytes[0];
        mem->data[i + 1u] = bytes[1];
        mem->data[i + 2u] = bytes[2];
        mem->data[i + 3u] = bytes[3];
        return 0;
    }

    if (!valid_external_word_address(address))
        return -1;

    return external_write(mem, address, bytes, sizeof bytes);
}

int t425_memory_write_masked_word(
    t425_memory_t *mem,
    uint32_t aligned_address,
    uint32_t value,
    uint32_t byte_mask)
{
    uint8_t bytes[4] = {
        (uint8_t)(value >> 0),
        (uint8_t)(value >> 8),
        (uint8_t)(value >> 16),
        (uint8_t)(value >> 24)
    };

    if (!mem || (aligned_address & 3u) != 0u ||
        byte_mask == 0u || byte_mask > 0x0Fu)
        return -1;

    if (valid_internal_word_address(aligned_address)) {
        uint32_t i = index_of(aligned_address);
        for (uint32_t lane = 0u; lane < 4u; ++lane) {
            if ((byte_mask & (UINT32_C(1) << lane)) != 0u)
                mem->data[i + lane] = bytes[lane];
        }
        return 0;
    }

    if (!valid_external_word_address(aligned_address))
        return -1;

    if (!mem->external_masked_write)
        return -1;

    return mem->external_masked_write(
        mem->external_context, aligned_address, value, byte_mask);
}
