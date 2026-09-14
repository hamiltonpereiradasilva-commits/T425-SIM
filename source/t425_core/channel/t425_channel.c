#include <stdint.h>

#include "t425_channel.h"

int t425_channel_read_word(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t *value)
{
    if (memory == 0 || value == 0)
        return -1;

    return t425_memory_read_word(
        memory,
        address,
        value);
}

int t425_channel_write_word(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t value)
{
    if (memory == 0)
        return -1;

    return t425_memory_write_word(
        memory,
        address,
        value);
}

int t425_channel_reset(
    t425_memory_t *memory,
    uint32_t address)
{
    return t425_channel_write_word(
        memory,
        address,
        T425_CHANNEL_EMPTY);
}

int t425_channel_is_empty(uint32_t value)
{
    return value == T425_CHANNEL_EMPTY;
}

int t425_channel_wait(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t wdesc)
{
    uint32_t current;

    if (memory == 0 || wdesc == T425_CHANNEL_EMPTY)
        return -1;

    if (t425_channel_read_word(
            memory,
            address,
            &current) != 0)
        return -1;

    /*
     * Only an empty channel can accept the first waiting process.
     */
    if (!t425_channel_is_empty(current))
        return -1;

    return t425_channel_write_word(
        memory,
        address,
        wdesc);
}

int t425_channel_take(
    t425_memory_t *memory,
    uint32_t address,
    uint32_t *wdesc)
{
    uint32_t current;

    if (memory == 0 || wdesc == 0)
        return -1;

    if (t425_channel_read_word(
            memory,
            address,
            &current) != 0)
        return -1;

    /*
     * An empty channel has no waiting process.
     */
    if (t425_channel_is_empty(current))
        return -1;

    /*
     * Rendezvous consumes the waiting Wdesc.
     */
    if (t425_channel_write_word(
            memory,
            address,
            T425_CHANNEL_EMPTY) != 0)
        return -1;

    *wdesc = current;

    return 0;
}
