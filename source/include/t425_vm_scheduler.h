#pragma once

#include <stdint.h>

#include "t425_vm.h"

/*
 * Resultado da execução de uma fatia da VM.
 *
 * O scheduler utiliza este resultado para decidir
 * se o processo volta para uma fila READY,
 * permanece bloqueado ou encerra.
 */

typedef enum
{
    T425_VM_SCHED_OK = 0,

    T425_VM_SCHED_YIELD,

    T425_VM_SCHED_BLOCKED,

    T425_VM_SCHED_HALTED,

    T425_VM_SCHED_FAULT

} t425_vm_sched_result_t;


/*
 * Executa um processo específico da VM.
 *
 * pid:
 *     processo arquitetural vm->proc[pid]
 *
 * budget:
 *     quantidade máxima de instruções
 */

t425_vm_sched_result_t
t425_vm_scheduler_execute(

    t425_vm_t *vm,

    uint32_t pid,

    uint32_t budget

);
