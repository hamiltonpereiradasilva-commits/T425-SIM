#ifndef T425_INSTRUCTION_H
#define T425_INSTRUCTION_H

#include <stdint.h>

typedef enum
{
    T425_OP_J     = 0x0,
    T425_OP_LDLP  = 0x1,
    T425_OP_PFIX  = 0x2,
    T425_OP_LDNL  = 0x3,
    T425_OP_LDC   = 0x4,
    T425_OP_LDNLP = 0x5,
    T425_OP_NFIX  = 0x6,
    T425_OP_LDL   = 0x7,
    T425_OP_ADC   = 0x8,
    T425_OP_CALL  = 0x9,
    T425_OP_CJ    = 0xA,
    T425_OP_AJW   = 0xB,
    T425_OP_EQC   = 0xC,
    T425_OP_STL   = 0xD,
    T425_OP_STNL  = 0xE,
    T425_OP_OPR   = 0xF
} t425_direct_opcode_t;

typedef struct
{
    uint8_t opcode;
    uint8_t operand_nibble;
} t425_instruction_t;

#endif
