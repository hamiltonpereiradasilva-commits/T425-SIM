#include <assert.h>
#include <stdio.h>
#include "t425_core.h"
int main(void)
{
    t425_core_t core;
    t425_core_reset(&core);
    assert(t425_memory_write_byte(&core.memory, 0x80000000u, 0x23u) == 0);
    assert(t425_memory_write_byte(&core.memory, 0x80000001u, 0x45u) == 0);
    core.cpu.Iptr = 0x80000000u;
    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(t425_core_step(&core) == T425_STEP_OK);
    assert(core.cpu.A == 0x35u);
    puts("F08.1-A prefix LDC #0x35: PASS");
    return 0;
}
