#include "t425_core_ifnext_34.h"

void t425_ifnext_conditions_34(
    const t425_ifnext_requests_34_t *r,
    t425_ifnext_conditions_34_t *c)
{
    if (c == 0) return;
    *c = (t425_ifnext_conditions_34_t){0};
    if (r == 0) return;
    c->ch_or_timer_req = (uint8_t)(
        r->channel_high || r->channel_low || r->timer_high || r->timer_low);
    c->ch_not_timer_req = (uint8_t)(r->channel_high || r->channel_low);
    c->high_not_low_req = (uint8_t)(r->channel_high || r->timer_high);
}

static t425_ifnext_result_34_t one_of(
    uint8_t channel, uint8_t timer, uint16_t channel_vector,
    uint16_t timer_vector, uint16_t *vector)
{
    if (channel && timer) return T425_IFNEXT_34_AMBIGUOUS;
    if (!channel && !timer) return T425_IFNEXT_34_NONE;
    if (vector != 0) *vector = channel ? channel_vector : timer_vector;
    return T425_IFNEXT_34_SELECTED;
}

t425_ifnext_result_34_t t425_ifnext_select_34(
    const t425_ifnext_requests_34_t *r,
    uint8_t current_process_low,
    uint8_t if_next,
    uint16_t *vector)
{
    t425_ifnext_result_34_t result;
    if (r == 0 || vector == 0 || !if_next) return T425_IFNEXT_34_NONE;
    *vector = 0u;

    /* High candidates take precedence. VBC names the four vectors; the
     * one-hot mapping is the explicit F34 experiment contract. */
    result = one_of(r->channel_high, r->timer_high, 0x1FBu, 0x1F8u, vector);
    if (result != T425_IFNEXT_34_NONE) return result;

    /* High-priority processes are not preempted by low requests. */
    if (!current_process_low) return T425_IFNEXT_34_NONE;

    return one_of(r->channel_low, r->timer_low, 0x1FAu, 0x1F9u, vector);
}
