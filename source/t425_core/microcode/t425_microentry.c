#include "t425_microentry.h"

uint16_t t425_microentry_from_instruction(
    uint8_t opcode,
    uint32_t oreg,
    uint16_t signal)
{
    /*
     * T425 CROM entry address.
     *
     * Normal opcode:
     *     MPC = 0x200 | opcode
     *
     * OPR:
     *     MPC = (Oreg | signal) & 0x3FF
     */
    if (opcode != 0x0Fu)
        return (uint16_t)(0x200u | (uint16_t)opcode);

    return (uint16_t)((oreg | (uint32_t)signal) & 0x03FFu);
}
