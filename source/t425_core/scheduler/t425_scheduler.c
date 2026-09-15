#include "t425_scheduler.h"

static uint32_t scheduler_priority(uint32_t wdesc)
{
    return wdesc & 1u;
}

static uint32_t scheduler_wptr(uint32_t wdesc)
{
    return wdesc & ~1u;
}

void t425_scheduler_reset(
    t425_scheduler_state_t *scheduler)
{
    if (!scheduler)
        return;

    *scheduler = (t425_scheduler_state_t){0};

    scheduler->fptr[T425_PRIORITY_HIGH] = T425_NOT_PROCESS;
    scheduler->bptr[T425_PRIORITY_HIGH] = T425_NOT_PROCESS;
    scheduler->fptr[T425_PRIORITY_LOW] = T425_NOT_PROCESS;
    scheduler->bptr[T425_PRIORITY_LOW] = T425_NOT_PROCESS;

    scheduler->current_wdesc = T425_NOT_PROCESS;
}

int t425_scheduler_enqueue(
    t425_scheduler_state_t *scheduler,
    t425_memory_t *memory,
    uint32_t wdesc)
{
    uint32_t priority;
    uint32_t wptr;
    uint32_t tail_wptr;
    uint32_t *fptr;
    uint32_t *bptr;

    if (!scheduler || !memory)
        return -1;

    if (wdesc == T425_NOT_PROCESS)
        return -1;

    priority = scheduler_priority(wdesc);

    if (priority > T425_PRIORITY_LOW)
        return -1;

    wptr = scheduler_wptr(wdesc);

    /*
     * Every enqueued process terminates its runnable-list link.
     */
    if (t425_memory_write_word(
            memory,
            wptr - 8u,
            T425_NOT_PROCESS) != 0)
        return -1;

    fptr = &scheduler->fptr[priority];
    bptr = &scheduler->bptr[priority];

    /*
     * Empty queue:
     *
     * FPtr[p] = Wdesc
     * BPtr[p] = Wdesc
     */
    if (*fptr == T425_NOT_PROCESS) {
        *fptr = wdesc;
        *bptr = wdesc;
        return 0;
    }

    /*
     * Non-empty queue:
     *
     * Memory[BPtr[p].Wptr - 8] = Wdesc
     * BPtr[p] = Wdesc
     */
    tail_wptr = scheduler_wptr(*bptr);

    if (t425_memory_write_word(
            memory,
            tail_wptr - 8u,
            wdesc) != 0)
        return -1;

    *bptr = wdesc;

    return 0;
}

int t425_scheduler_dequeue(
    t425_scheduler_state_t *scheduler,
    t425_memory_t *memory,
    uint32_t *wdesc,
    uint32_t *iptr)
{
    uint32_t priority;
    uint32_t selected;
    uint32_t selected_wptr;
    uint32_t next;

    if (!scheduler || !memory || !wdesc || !iptr)
        return -1;

    /*
     * Architectural priority selection:
     * HIGH is selected before LOW.
     */
    if (scheduler->fptr[T425_PRIORITY_HIGH] != T425_NOT_PROCESS)
        priority = T425_PRIORITY_HIGH;
    else if (scheduler->fptr[T425_PRIORITY_LOW] != T425_NOT_PROCESS)
        priority = T425_PRIORITY_LOW;
    else {
        *wdesc = T425_NOT_PROCESS;
        *iptr = 0u;
        return 1;
    }

    selected = scheduler->fptr[priority];
    selected_wptr = scheduler_wptr(selected);

    /*
     * Iptr is architecturally stored at Wptr - 4.
     */
    if (t425_memory_read_word(
            memory,
            selected_wptr - 4u,
            iptr) != 0)
        return -1;

    /*
     * Last element:
     *
     * FPtr[p] = NOT_PROCESS
     *
     * BPtr[p] is deliberately NOT cleared.
     */
    if (scheduler->fptr[priority] == scheduler->bptr[priority]) {
        scheduler->fptr[priority] = T425_NOT_PROCESS;
    } else {
        /*
         * FIFO successor is stored at Wptr - 8.
         */
        if (t425_memory_read_word(
                memory,
                selected_wptr - 8u,
                &next) != 0)
            return -1;

        scheduler->fptr[priority] = next;
    }

    *wdesc = selected;
    return 0;
}
