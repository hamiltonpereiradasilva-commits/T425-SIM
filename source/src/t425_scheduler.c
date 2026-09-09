#include "t425_scheduler.h"
#include "t425_vm_scheduler.h"

#include <string.h>
#include <stdio.h>


static bool queue_push(
    uint32_t *queue,
    uint32_t *tail,
    uint32_t *count,
    uint32_t pid
)
{
    if (*count >= T425_MAX_PROCESSES)
        return false;

    queue[*tail] = pid;

    *tail =
        (*tail + 1) %
        T425_MAX_PROCESSES;

    (*count)++;

    return true;
}


static bool queue_pop(
    uint32_t *queue,
    uint32_t *head,
    uint32_t *count,
    uint32_t *pid
)
{
    if (*count == 0)
        return false;

    *pid = queue[*head];

    *head =
        (*head + 1) %
        T425_MAX_PROCESSES;

    (*count)--;

    return true;
}


bool t425_scheduler_init(
    t425_scheduler_t *s
)
{
    if (!s)
        return false;

    memset(s, 0, sizeof(*s));

    s->state =
        T425_SCHED_READY;

    s->current_pid =
        UINT32_MAX;

    return true;
}


bool t425_scheduler_start(
    t425_scheduler_t *s
)
{
    if (!s)
        return false;

    if (s->state !=
        T425_SCHED_READY)
        return false;

    s->state =
        T425_SCHED_RUNNING;

    return true;
}


bool t425_scheduler_enqueue(
    t425_scheduler_t *s,
    uint32_t pid,
    uint32_t priority
)
{
    if (!s)
        return false;

    if (pid >=
        T425_MAX_PROCESSES)
        return false;

    /*
     * Prioridade 0:
     * baixa prioridade.
     *
     * Prioridade != 0:
     * alta prioridade.
     */

    if (priority)
    {
        return queue_push(
            s->high_queue,
            &s->high_tail,
            &s->high_count,
            pid
        );
    }

    return queue_push(
        s->low_queue,
        &s->low_tail,
        &s->low_count,
        pid
    );
}


bool t425_scheduler_dequeue(
    t425_scheduler_t *s,
    uint32_t *pid
)
{
    if (!s || !pid)
        return false;

    /*
     * Processo de alta prioridade
     * sempre vence.
     */

    if (s->high_count)
    {
        return queue_pop(
            s->high_queue,
            &s->high_head,
            &s->high_count,
            pid
        );
    }

    if (s->low_count)
    {
        return queue_pop(
            s->low_queue,
            &s->low_head,
            &s->low_count,
            pid
        );
    }

    return false;
}


bool t425_scheduler_step(

    t425_scheduler_t *s,

    t425_runtime_t *rt

)

{

    /* T425_RUNTIME_TRACE_SCHEDULER */
    static uint32_t t425_trace_sched_calls = 0;

    t425_trace_sched_calls++;

    if ((t425_trace_sched_calls % 100u) == 0u) {

        printf(
            "[TRACE SCHED] "
            "calls=%lu\n",

            (unsigned long)t425_trace_sched_calls
        );
    }

    if (!s ||
        !rt ||
        s->state !=
        T425_SCHED_RUNNING)
        return false;


    /*
     * Clock do runtime.
     */

    t425_runtime_tick(rt);

    s->cycles++;

    s->timer_events++;


    /*
     * Nesta versão o scheduler mantém
     * a compatibilidade com a API antiga.
     *
     * A seleção e execução da VM é feita
     * através da função de integração
     * chamada pelo supervisor.
     *
     * O scheduler continua responsável
     * pelas filas e contabilização.
     */

    return true;
}

bool t425_scheduler_fault(
    t425_scheduler_t *s
)
{
    if (!s)
        return false;

    s->state =
        T425_SCHED_FAULT;

    s->fault_events++;

    return true;
}
