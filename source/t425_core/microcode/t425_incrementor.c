#include "t425_incrementor.h"

void t425_incrementor_reset(t425_incrementor_result_t *result)
{
    if (result == 0)
        return;

    result->inc_oreg_carryout = 0u;
    result->inc_dreg_carryout = 0u;
}

static uint8_t increment_word(uint32_t *value)
{
    uint32_t old_value;
    uint32_t new_value;

    if (value == 0)
        return 0u;

    old_value = *value;
    new_value = old_value + 4u;
    *value = new_value;

    return (new_value < old_value) ? 1u : 0u;
}

void t425_incrementor_word_oreg(
    uint32_t *oreg,
    t425_incrementor_result_t *result)
{
    if (result == 0)
        return;

    result->inc_oreg_carryout = increment_word(oreg);
}

void t425_incrementor_word_dreg(
    uint32_t *dreg,
    t425_incrementor_result_t *result)
{
    if (result == 0)
        return;

    result->inc_dreg_carryout = increment_word(dreg);
}
