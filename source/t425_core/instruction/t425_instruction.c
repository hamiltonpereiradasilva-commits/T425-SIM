#include <stdint.h>

#include "t425_core.h"
#include "t425_channel.h"
#include "t425_process.h"
#include "t425_instruction.h"
#include "t425_timer.h"

int t425_core_step(t425_core_t *core)
{
    t425_cpu_state_t *cpu;
    uint8_t instruction;
    uint8_t opcode;
    uint8_t nibble;

    if (!core)
        return T425_STEP_BADMEM;

    if (t425_core_system_step(core))
        return T425_STEP_OK;

    cpu = &core->cpu;

    if (cpu->halted)
        return T425_STEP_HALTED;

    /*
     * Persistent instruction/microcode execution state.
     *
     * Once FETCH has established the physical microcode entry,
     * subsequent calls execute exactly one microcycle.
     *
     * Next is handled by t425_core_microcycle(), which updates
     * next_step_instruction from the current MIR interface request.
     */
    if (core->step_instruction != 0u) {
        t425_core_microcycle(core);

        core->step_instruction =
            core->next_step_instruction;

        core->cycles += 1u;

        if (core->step_instruction == 0u)
            core->instructions += 1u;

        return T425_STEP_OK;
    }

    if (t425_memory_read_byte(
            &core->memory, cpu->IptrReg, &instruction) != 0)
        return T425_STEP_BADMEM;

    cpu->IptrReg += 1u;

    opcode = (uint8_t)(instruction >> 4);
    nibble = (uint8_t)(instruction & 0x0Fu);

    cpu->Oreg = (cpu->Oreg & 0xFFFFFFF0u) | nibble;

    /*
     * F11.6 — enter the established T425 microcode machine.
     *
     * FETCH has already supplied:
     *   - opcode
     *   - low nibble in Oreg
     *
     * The physical entry is:
     *   opcode -> MPC -> CROM -> MIR -> microsequencer.
     */
    /*
     * Enter the established physical T425 microcode machine.
     * The microsequencer owns microstate progression.
     */
    t425_core_microentry(core, opcode);

    /*
     * FETCH has completed and the instruction is now executing
     * inside the established T425 microcode machine.
     *
     * The first microcycle is intentionally performed by the
     * next invocation of t425_core_step().
     */
    core->step_instruction = 1u;
    core->next_step_instruction = 1u;

    return T425_STEP_OK;
}
