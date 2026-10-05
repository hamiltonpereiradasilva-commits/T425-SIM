#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "t425_core.h"
#include "t425_channel.h"
int main(void)
{
    t425_core_t c = {0};
    uint32_t code = T425_INTERNAL_RAM_BASE;
    t425_core_reset(&c);
    (void)t425_memory_write_byte(&c.memory, code, 0x20u);
    (void)t425_memory_write_byte(&c.memory, code + 1u, 0xF7u);
    c.cpu.IptrReg = code;
    c.cpu.A = 4u;
    c.cpu.B = 0x80000010u;
    c.cpu.C = 0x80000200u;
    (void)t425_channel_wait(&c.memory, 0x80000010u, 0x80000301u);
    for (unsigned i = 0u; i < 80u; ++i)
        (void)t425_core_step(&c);
    if (c.external_in_active[0] == 0u ||
        c.external_in_remaining[0] != 4u) {
        fprintf(stderr,
                "REAL_EXTERNAL_IN_PATH=NOT_REACHED active=%u remaining=%u mpc=0x%03X\n",
                c.external_in_active[0],
                c.external_in_remaining[0],
                c.microsequencer.mpc);
        return 1;
    }
    assert(c.external_in_active[1] == 0u);
    assert(c.external_in_active[2] == 0u);
    assert(c.external_in_active[3] == 0u);
    puts("F_PHY-71E3 real OPR IN lifetime: PASS");
    return 0;
}
