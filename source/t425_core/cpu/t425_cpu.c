#include "t425_cpu.h"

void t425_cpu_reset(t425_cpu_state_t *cpu)
{
    if (!cpu)
        return;

    *cpu = (t425_cpu_state_t){0};
    cpu->ASlave = cpu->A;
    cpu->BSlave = cpu->B;
    cpu->CSlave = cpu->C;
}
