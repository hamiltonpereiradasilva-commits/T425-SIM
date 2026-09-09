#include "t425_supervisor.h"
#include <stdio.h>

bool t425_supervisor_init(
    t425_supervisor_t *s,
    const t425_node_config_t *c
){
    if(!s || !c)
        return false;

    s->boot_count++;

    if(!t425_runtime_init(&s->runtime, c, 3, 10))
        return false;

    /*
     * Inicializa a VM arquitetural.
     */
    /*
     * Processo 0 existe como processo inicial.
     */
    /*
     * Inicializa o scheduler.
     */

    return t425_scheduler_init(&s->scheduler);
}

bool t425_supervisor_start(t425_supervisor_t *s)
{
    if(!s)
        return false;

    /*
     * START idempotente:
     * se já estiver em execução, não é erro.
     */
    if(s->scheduler.state == T425_SCHED_RUNNING)
        return true;

    /*
     * Só podemos iniciar a partir de READY.
     */
    if(s->scheduler.state != T425_SCHED_READY)
        return false;

    if(!t425_scheduler_start(&s->scheduler))
        return false;

    s->runtime.state = T425_RT_RUNNING;

    return true;
}

bool t425_supervisor_step(t425_supervisor_t *s)
{
    if(!s)
        return false;

    /*
     * Não executar o runtime antes do START.
     */
    if(s->scheduler.state != T425_SCHED_RUNNING)
        return false;

    return t425_scheduler_step(
        &s->scheduler,
        &s->runtime
    );
}

bool t425_supervisor_status(
    const t425_supervisor_t *s,
    char *b,
    uint32_t cap
){
    if(!s || !b || cap == 0)
        return false;

    int n = snprintf(
        b,
        cap,
        "NODE=%u RT=%d SCHED=%d TICKS=%lu RX=%lu TX=%lu BAD=%lu",
        s->runtime.config.node_id,
        (int)s->runtime.state,
        (int)s->scheduler.state,
        (unsigned long)s->runtime.ticks,
        (unsigned long)s->runtime.frames_rx,
        (unsigned long)s->runtime.frames_tx,
        (unsigned long)s->runtime.frames_bad
    );

    return n >= 0 && (uint32_t)n < cap;
}
