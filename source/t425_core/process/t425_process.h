#ifndef T425_CORE_PROCESS_H
#define T425_CORE_PROCESS_H

#include <stdint.h>

typedef uint32_t t425_wdesc_t;

#define T425_PRIORITY_HIGH 0u
#define T425_PRIORITY_LOW  1u

#define T425_WDESC_PRIORITY_MASK 1u
#define T425_WDESC_WPTR_MASK (~1u)

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
