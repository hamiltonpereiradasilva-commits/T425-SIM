#include "t425_byte_write.h"

#include <limits.h>

static int32_t negate_i32_modulo(uint32_t value)
{
    int32_t signed_value = (int32_t)value;
    if (signed_value == INT32_MIN)
        return INT32_MIN;
    return -signed_value;
}

/* Compact translation of EMUL64 _DetermineByteWriteStrobes. */
uint8_t t425_byte_write_lane_mask(
    uint32_t areg, uint32_t breg, uint32_t dreg,
    uint8_t mir_strobe_enable, uint8_t strobe_latch)
{
    int32_t a = (int32_t)areg;
    int32_t d = (int32_t)dreg;
    uint32_t offset = breg & 3u;
    int32_t count;
    uint32_t first_lane;
    uint32_t lane_count;
    uint8_t mask = 0u;

    if (offset == 1u)
        count = a <= 2 ? a : negate_i32_modulo(dreg);
    else if (offset == 2u && a == 1)
        count = a;
    else
        count = negate_i32_modulo(dreg);

    if ((mir_strobe_enable & 1u) == 0u && (strobe_latch & 1u) == 0u)
        return 0x0Fu;

    first_lane = (mir_strobe_enable & 1u) != 0u ? offset : 0u;

    /* The pseudocode's A == -3, D == -3 branches keep the initial lane. */
    if (a == -3 && d == -3)
        count = 1;

    /* In the decoded zero-count case, lane 3 remains selected. */
    if (count == 0)
        return first_lane == 3u ? 0x08u : 0u;

    /* Negative sentinel counts retain the initially selected lane. */
    lane_count = count < 0 ? 1u : (uint32_t)count;
    if (lane_count > 4u - first_lane)
        lane_count = 4u - first_lane;
    for (uint32_t i = 0u; i < lane_count; ++i)
        mask |= (uint8_t)(1u << (first_lane + i));
    return mask;
}
