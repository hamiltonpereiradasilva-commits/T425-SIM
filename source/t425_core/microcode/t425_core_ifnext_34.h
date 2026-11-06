#ifndef T425_CORE_IFNEXT_34_H
#define T425_CORE_IFNEXT_34_H
#include <stdint.h>

typedef struct {
    uint8_t channel_high;
    uint8_t channel_low;
    uint8_t timer_high;
    uint8_t timer_low;
} t425_ifnext_requests_34_t;

typedef struct {
    uint8_t ch_or_timer_req;
    uint8_t ch_not_timer_req;
    uint8_t high_not_low_req;
} t425_ifnext_conditions_34_t;

typedef enum {
    T425_IFNEXT_34_NONE = 0,
    T425_IFNEXT_34_SELECTED,
    T425_IFNEXT_34_AMBIGUOUS
} t425_ifnext_result_34_t;

void t425_ifnext_conditions_34(
    const t425_ifnext_requests_34_t *requests,
    t425_ifnext_conditions_34_t *conditions);

/* `current_process_low` is sampled from the real process-priority state.
 * A high-priority process admits only high requests; a low-priority process
 * admits either class. Multiple equal-priority candidates are reported as
 * ambiguous and left unserviced until a documented tie-break is available. */
t425_ifnext_result_34_t t425_ifnext_select_34(
    const t425_ifnext_requests_34_t *requests,
    uint8_t current_process_low,
    uint8_t if_next,
    uint16_t *vector);

#endif
