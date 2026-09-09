#include <stdio.h>
#include "t425_debug.h"

void t425_debug_print_state(const t425_vm_t *vm) {
    printf("NODE %d PC=%08lx A=%08lx B=%08lx C=%08lx W=%08lx I=%08lx INS=%llu HALT=%d ERR=%d\n",
        T425_NODE_ID,
        (unsigned long)vm->cpu.PC, (unsigned long)vm->cpu.A,
        (unsigned long)vm->cpu.B, (unsigned long)vm->cpu.C,
        (unsigned long)vm->cpu.W, (unsigned long)vm->cpu.I,
        (unsigned long long)vm->instructions,
        vm->cpu.halted, vm->cpu.error);
}
