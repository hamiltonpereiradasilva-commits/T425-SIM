#include <assert.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_clock.h"
#include "t425_instruction.h"
#include "t425_process.h"

int main(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    assert(core.cpu.A == 0);
    assert(core.cpu.B == 0);
    assert(core.cpu.C == 0);

    assert(core.cpu.Wptr == 0);
    assert(core.cpu.Iptr == 0);

    assert(core.cycles == 0);
    assert(core.instructions == 0);

    assert(T425_CLOCK_HZ == 20000000u);
    assert(T425_CYCLE_NS == 50u);

    assert(T425_OP_J == 0x0);
    assert(T425_OP_LDLP == 0x1);
    assert(T425_OP_PFIX == 0x2);
    assert(T425_OP_OPR == 0xF);

    {
        t425_wdesc_t high =
            0x00001000u | T425_PRIORITY_HIGH;

        t425_wdesc_t low =
            0x00002000u | T425_PRIORITY_LOW;

        assert(t425_wdesc_wptr(high) == 0x00001000u);
        assert(t425_wdesc_priority(high) ==
               T425_PRIORITY_HIGH);

        assert(t425_wdesc_wptr(low) == 0x00002000u);
        assert(t425_wdesc_priority(low) ==
               T425_PRIORITY_LOW);
    }

    puts("F07.1 FOUNDATION TEST: PASS");

    return 0;
}
