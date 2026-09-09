#include "t425_instruction.h"

int t425_instruction_foundation_anchor(void)
{
    return (T425_OP_OPR == 0xF) ? 0 : -1;
}
