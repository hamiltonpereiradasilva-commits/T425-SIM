#include "t425_vm_scheduler.h"

/*
 * A VM atual possui:
 *
 *     vm->current_pid
 *
 * e:
 *
 *     vm->proc[pid]
 *
 * Portanto o scheduler seleciona o PID e a VM
 * executa diretamente aquele processo.
 */


t425_vm_sched_result_t
t425_vm_scheduler_execute(

    t425_vm_t *vm,

    uint32_t pid,

    uint32_t budget

)

{
    if (!vm)
        return T425_VM_SCHED_FAULT;

    if (pid >= T425_MAX_PROCESSES)
        return T425_VM_SCHED_FAULT;


    /*
     * Processo selecionado pelo scheduler.
     */

    vm->current_pid = pid;


    /*
     * Não executar processos livres.
     */

    if (vm->proc[pid].state == PROC_FREE)
        return T425_VM_SCHED_HALTED;


    /*
     * Executa uma fatia limitada.
     */

    if (budget > 0)
        t425_vm_run_slice(

            vm,

            budget

        );


    /*
     * CPU global parada.
     */

    if (vm->cpu.halted)
        return T425_VM_SCHED_HALTED;


    /*
     * Interpretação do estado arquitetural
     * do processo após a execução.
     *
     * READY:
     *     volta ao scheduler.
     *
     * FREE:
     *     processo encerrado.
     *
     * Outros estados:
     *     tratado como bloqueado até o runtime
     *     acordá-lo novamente.
     */

    if (vm->proc[pid].state == PROC_READY)
        return T425_VM_SCHED_YIELD;


    if (vm->proc[pid].state == PROC_FREE)
        return T425_VM_SCHED_HALTED;


    return T425_VM_SCHED_BLOCKED;
}
