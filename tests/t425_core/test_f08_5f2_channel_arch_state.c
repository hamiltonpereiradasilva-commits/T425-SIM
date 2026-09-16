#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

int main(void)
{
    t425_core_t core;

    t425_core_reset(&core);

    assert(core.t425_channel_select == 0u);
    assert(core.t425_channel_stack == 0u);
    assert(core.t425_data_mem_addr == 0u);
    assert(core.t425_data_reg == 0u);

    /*
     * ChannelDataReg is architecturally distinct from DataReg.
     */
    assert(core.t425_channel_data_reg == 0u);

    /*
     * VBC channel-control state starts reset.
     */
    assert(core.t425_channel_ready_not_run == 0u);
    assert(core.t425_channel_run_req == 0u);
    assert(core.t425_channel_status == 0u);
    assert(core.t425_alt_enabled == 0u);

    /*
     * Verify that DataReg and ChannelDataReg are independent state.
     */
    core.t425_data_reg = 0x11223344u;
    core.t425_channel_data_reg = 0xAABBCCDDu;

    assert(core.t425_data_reg == 0x11223344u);
    assert(core.t425_channel_data_reg == 0xAABBCCDDu);

    /*
     * Verify the architectural channel datapath registers are writable.
     */
    core.t425_channel_select = 0x8000001Cu;
    core.t425_channel_stack = 0x80000070u;

    assert(core.t425_channel_select == 0x8000001Cu);
    assert(core.t425_channel_stack == 0x80000070u);

    /*
     * Verify channel-control state is independently representable.
     */
    core.t425_channel_ready_not_run = 1u;
    core.t425_channel_run_req = 1u;
    core.t425_channel_status = 1u;
    core.t425_alt_enabled = 1u;

    assert(core.t425_channel_ready_not_run == 1u);
    assert(core.t425_channel_run_req == 1u);
    assert(core.t425_channel_status == 1u);
    assert(core.t425_alt_enabled == 1u);

    puts("F08.5-F.2 PASS: architectural channel state is distinct and initialized");

    return 0;
}
