#ifndef T425_CORE_PROCESS_H
#define T425_CORE_PROCESS_H

#include <stdint.h>

typedef uint32_t t425_wdesc_t;

#define T425_PRIORITY_HIGH 0u
#define T425_PRIORITY_LOW  1u

#define T425_WDESC_PRIORITY_MASK 1u
#define T425_WDESC_WPTR_MASK (~1u)

#define T425_PROCESS_IPTR_OFFSET_BYTES   4u
#define T425_PROCESS_LINK_OFFSET_BYTES   8u
#define T425_PROCESS_STATE_OFFSET_BYTES 12u
#define T425_PROCESS_TLINK_OFFSET_BYTES 16u
#define T425_PROCESS_TIME_OFFSET_BYTES  20u

#define T425_PROCESS_STATE_NOT_PROCESS 0x80000000u
#define T425_PROCESS_STATE_ENABLING    0x80000001u
#define T425_PROCESS_STATE_WAITING     0x80000002u
#define T425_PROCESS_STATE_READY       0x80000003u
#define T425_TIMER_TIME_SET             0x80000001u
#define T425_TIMER_TIME_NO_SET          0x80000002u

static inline uint32_t
t425_wdesc_wptr(t425_wdesc_t wdesc)
{
    return wdesc & T425_WDESC_WPTR_MASK;
}

static inline uint32_t
t425_wdesc_priority(t425_wdesc_t wdesc)
{
    return wdesc & T425_WDESC_PRIORITY_MASK;
}

#endif
