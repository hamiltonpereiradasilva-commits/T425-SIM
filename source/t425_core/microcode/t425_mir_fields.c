#include "t425_mir_fields.h"

#include <stddef.h>

static uint32_t bit(const t425_mir_t *mir, unsigned n)
{
    return (mir != NULL) ? (uint32_t)(mir->bit[n] & 1u) : 0u;
}

static uint32_t field(
    const t425_mir_t *mir,
    const unsigned *positions,
    unsigned count
)
{
    uint32_t value = 0u;

    for (unsigned i = 0u; i < count; ++i)
        value = (value << 1) | bit(mir, positions[i]);

    return value;
}

/*
 * Positions are listed from most significant field bit
 * to least significant field bit, exactly as documented.
 */

uint16_t t425_mir_xbus(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {99,97,78,75,73,70,53,52,46,44};

    return (uint16_t)field(mir, p, 10u);
}

uint16_t t425_mir_ybus(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {105,104,96,80,66,59,58,50,48,42,37,35,20};

    return (uint16_t)field(mir, p, 13u);
}

uint16_t t425_mir_alu(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {21,19,18,17,16,15,14};

    return (uint16_t)field(mir, p, 7u);
}

uint32_t t425_mir_zbus(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {
            103,98,79,77,69,65,57,51,49,47,
            45,43,41,38,36,34,12,8,0
        };

    return field(mir, p, 19u);
}

uint8_t t425_mir_stack(const t425_mir_t *mir)
{
    static const unsigned p[] = {67,64,60,56,54};
    return (uint8_t)field(mir, p, 5u);
}

uint8_t t425_mir_shift(const t425_mir_t *mir)
{
    static const unsigned p[] = {68,63,62,61,55,23,22};
    return (uint8_t)field(mir, p, 7u);
}

uint8_t t425_mir_cond0(const t425_mir_t *mir)
{
    static const unsigned p[] = {27,26,25};
    return (uint8_t)field(mir, p, 3u);
}

uint8_t t425_mir_cond1(const t425_mir_t *mir)
{
    static const unsigned p[] = {30,28,32,31};
    return (uint8_t)field(mir, p, 4u);
}

uint8_t t425_mir_timer(const t425_mir_t *mir)
{
    static const unsigned p[] = {39,33,13};
    return (uint8_t)field(mir, p, 3u);
}

uint8_t t425_mir_incrementor(const t425_mir_t *mir)
{
    static const unsigned p[] = {102,101,100,76};
    return (uint8_t)field(mir, p, 4u);
}

uint8_t t425_mir_interface(const t425_mir_t *mir)
{
    static const unsigned p[] = {109,106,83,82,81,72,71};
    return (uint8_t)field(mir, p, 7u);
}

uint8_t t425_mir_byte(const t425_mir_t *mir)
{
    static const unsigned p[] = {108,107};
    return (uint8_t)field(mir, p, 2u);
}

uint8_t t425_mir_error(const t425_mir_t *mir)
{
    return (uint8_t)bit(mir, 40u);
}

uint8_t t425_mir_channel(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {74,11,10,9,7,6,5,4,3,2,1};

    return (uint8_t)field(mir, p, 11u);
}

uint16_t t425_mir_next_field(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {117,116,115,114,113,112,111,110,29,24};

    return (uint16_t)field(mir, p, 10u);
}

uint16_t t425_mir_const(const t425_mir_t *mir)
{
    static const unsigned p[] =
        {95,94,93,92,91,90,89,88,87,86,85,84};

    return (uint16_t)field(mir, p, 12u);
}
