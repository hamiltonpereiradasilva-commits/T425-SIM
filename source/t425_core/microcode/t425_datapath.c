#include "t425_mir_fields.h"
#include "t425_datapath.h"

#include <stddef.h>

/*
 * XBUS:
 * Microword[99,97,78,75,73,70,53:52,46,44]
 */
static const unsigned xbus_bits[10] = {
    99u, 97u, 78u, 75u, 73u,
    70u, 53u, 52u, 46u, 44u
};

/*
 * YBUS:
 * Microword[105:104,96,80,66,59:58,50,48,42,37,35,20]
 */
static const unsigned ybus_bits[13] = {
    105u, 104u, 96u, 80u, 66u,
    59u, 58u, 50u, 48u, 42u,
    37u, 35u, 20u
};

/*
 * ALU:
 * Microword[21,19:14]
 */
static const unsigned alu_bits[7] = {
    21u, 19u, 18u, 17u, 16u, 15u, 14u
};

/*
 * ZBUS destination:
 * Microword[103,98,79,77,69,65,57,51,49,47,
 *           45,43,41,38,36,34,12,8,0]
 */
static const unsigned zbus_bits[19] = {
    103u, 98u, 79u, 77u, 69u,
    65u, 57u, 51u, 49u, 47u,
    45u, 43u, 41u, 38u, 36u,
    34u, 12u, 8u, 0u
};

/*
 * Constant Box:
 * Microword[95:84]
 */
static const unsigned constbox_bits[12] = {
    95u, 94u, 93u, 92u, 91u, 90u,
    89u, 88u, 87u, 86u, 85u, 84u
};

static uint32_t decode_field(
    const t425_mir_t *mir,
    const unsigned *bits,
    size_t count)
{
    uint32_t value = 0u;
    size_t i;

    for (i = 0u; i < count; ++i) {
        if (mir->bit[bits[i]] != 0u) {
            value |= (uint32_t)1u << (count - 1u - i);
        }
    }

    return value;
}

/*
 * Constant-box encodings taken directly from t425revC.vbc.
 */
/*
 * T425 A/B/C stack control:
 * Microword[67,64,60,56,54]
 *
 * 10000 AregfromBregSlave
 * 01000 BregfromAregSlave
 * 00100 BregfromCregSlave
 * 00010 CregfromBregSlave
 * 00001 CregFrom0
 */
static uint32_t decode_stack_control(const t425_mir_t *mir)
{
    static const unsigned stack_bits[5] = {
        67u, 64u, 60u, 56u, 54u
    };

    return decode_field(
        mir,
        stack_bits,
        sizeof(stack_bits) / sizeof(stack_bits[0]));
}

/*
 * T425 A/B/C shift control:
 * Microword[68,63:61,55,23:22]
 *
 * 68  ShiftRightAreg
 * 63  ShiftLeftBreg
 * 62  BregShIn0From1
 * 61  ShiftRightBreg
 * 55  CregFromZbusSR1
 * 23  CregShInMsbfromCarryOut
 * 22  CregShInMsbfromArithLogic
 */
static uint32_t decode_shift_control(const t425_mir_t *mir)
{
    static const unsigned shift_bits[7] = {
        68u, 63u, 62u, 61u, 55u, 23u, 22u
    };

    return decode_field(
        mir,
        shift_bits,
        sizeof(shift_bits) / sizeof(shift_bits[0]));
}

static uint32_t decode_constbox(const t425_mir_t *mir)
{
    uint32_t selector;

    selector = decode_field(
        mir,
        constbox_bits,
        sizeof(constbox_bits) / sizeof(constbox_bits[0]));

    switch (selector) {
        case 0x000u: return 0xFFFFFFFFu; /* Minus1 */
        case 0x800u: return 0xFFFFFFFEu; /* Minus2 */
        case 0x400u: return 0xFFFFFFFDu; /* Minus3 */
        case 0xC00u: return 0xFFFFFFFCu; /* Minus4 */
        case 0x200u: return 0xFFFFFFFBu; /* Minus5 */

        case 0xFFEu: return 0x80000000u; /* MinInt */
        case 0x7FEu: return 0x80000001u;
        case 0xBFEu: return 0x80000002u;
        case 0x3FEu: return 0x80000003u;

        case 0xFFFu: return 0x00000000u; /* Zero */
        case 0x7FFu: return 0x00000001u; /* One */
        case 0xBFFu: return 0x00000002u; /* Two */
        case 0x3FFu: return 0x00000003u; /* Three */
        case 0xDFFu: return 0x00000004u; /* Four */
        case 0x5FFu: return 0x00000005u; /* Five */
        case 0xEFFu: return 0x00000008u; /* Eight */
        case 0xCFFu: return 0x0000000Cu; /* Twelve */
        case 0xD7Fu: return 0x00000014u; /* Twenty */
        case 0xF7Fu: return 0x00000010u; /* Sixteen */
        case 0xFDFu: return 0x00000020u; /* ThirtyTwo */
        case 0xDBFu: return 0x00000024u; /* ThirtySix */
        case 0xE3Fu: return 0x00000038u; /* FiftySix */
        case 0xF5Fu: return 0x00000040u; /* SixtyFour */
        case 0xDDFu: return 0x00000084u; /* HundredThirtyTwo */
        case 0xC1Fu: return 0x0000007Cu; /* HundredTwentyFour */
        case 0x01Fu: return 0x0000007Fu; /* HundredTwentySeven */

        case 0xFBEu: return 0x80000020u; /* Event */
        case 0xEDEu: return 0x80000048u; /* ExtendFunctions */
        case 0xC7Eu: return 0x8000001Cu; /* Link3Input */
        case 0x00Fu: return 0x000000FFu; /* ByteMask */

        case 0xF1Eu: return 0x80000070u; /* MemStart */
        case 0x71Eu: return 0x80000071u; /* MemStart+1 */
        case 0xE1Eu: return 0x80000078u; /* MemStart+8 */

        case 0xEBEu: return 0x80000028u; /* TptrLoc1 */
        case 0xDBEu: return 0x80000024u; /* TptrLoc0 */
        case 0xDDEu: return 0x80000044u; /* EregIntSaveLoc */
        case 0xCFEu: return 0x8000000Cu; /* Link3Output */
        case 0xF7Eu: return 0x80000040u; /* StatusIntSaveLoc */
        case 0xF3Eu: return 0x80000030u; /* IptrIntSaveLoc */
        case 0xD3Eu: return 0x80000034u; /* AregSaveLoc */
        case 0xE3Eu: return 0x80000038u; /* BregSaveLoc */
        case 0xC3Eu: return 0x8000003Cu; /* CregSaveLoc */
        case 0xC5Eu: return 0x8000005Cu; /* CRCFunctionLoc */
        case 0xCBEu: return 0x8000002Cu; /* WdescIntSaveLoc */

        case 0xFFD:
            return 0x7F800000u; /* PositiveInfinity */

        case 0x001u: return 0x7FFFFFFFu; /* MostPos */
        case 0x801u: return 0x7FFFFFFEu; /* MostPosMinus1 */

        case 0xE00u: return 0xFFFFFFF8u; /* Minus8 */
        case 0xD00u: return 0xFFFFFFF4u; /* Minus12 */
        case 0xF00u: return 0xFFFFFFF0u; /* Minus16 */
        case 0xC80u: return 0xFFFFFFECu; /* Minus20 */
        case 0x680u: return 0xFFFFFFE9u; /* Minus23 */

        default:
            return 0u;
    }
}


/*
 * F20-05E
 *
 * Reconstruction of the original T425 ByteUpShifter decision tree.
 *
 * Evidence:
 *   t425emu_lite.exe::_ByteUpShifter
 *   0x407b80 .. 0x407d71
 *
 * IMPORTANT:
 * This is intentionally kept separate from DataRegfromByteShifter.
 * The helper only reproduces the ByteUpShifter combinational result.
 */
static uint32_t t425_byte_upshifter(
    uint32_t areg,
    uint32_t breg,
    uint32_t creg,
    uint32_t data_in,
    uint32_t previous_read,
    uint8_t latch_enable)
{
    uint32_t a = areg & 0x3u;

    /*
     * LATCH == 0
     *
     * 0x407b89 .. 0x407bb5
     */
    if (latch_enable == 0u) {
        switch (a) {
        case 0u:
            return data_in;

        case 1u:
            return data_in >> 8;

        case 2u:
            return data_in & 0x0000FFFFu;

        default:
            return data_in & 0x000000FFu;
        }
    }

    /*
     * LATCH == 1
     *
     * 0x407bc7 .. 0x407be4
     */
    uint32_t b = breg & 0x3u;
    uint32_t c = creg & 0x3u;

    int32_t d = (int32_t)b - (int32_t)c;

    if (d < 0)
        d += 4;

    /*
     * Special path corresponding to 0x407bf3 .. 0x407c07:
     *
     * C == 1
     * B == 0
     * AReg-1 <= 2
     *
     * i.e. AReg in [1,3].
     */
    if (c == 1u && b == 0u &&
        areg >= 1u && areg <= 3u) {

        if (d == 0)
            return data_in;

        if (d == 3)
            return data_in >> 8;

        if (d == 2)
            return data_in & 0x0000FFFFu;

        return data_in & 0x000000FFu;
    }

    /*
     * 0x407c40 .. 0x407c5f
     *
     * C == 2
     * B != 0
     * AReg-1 <= 1
     *
     * The actual branch enters the common special selector.
     */
    if (c == 2u && b != 0u &&
        areg >= 1u && areg <= 2u) {

        if (d == 0)
            return data_in;

        if (d == 3)
            return data_in >> 8;

        if (d == 2)
            return data_in & 0x0000FFFFu;

        return data_in & 0x000000FFu;
    }

    /*
     * C == 3
     * B != 0
     * AReg == 1
     *
     * 0x407c61 .. 0x407c74
     */
    if (c == 3u && b != 0u && areg == 1u) {

        if (d == 0)
            return data_in;

        if (d == 3)
            return data_in >> 8;

        if (d == 2)
            return data_in & 0x0000FFFFu;

        return data_in & 0x000000FFu;
    }

    /*
     * Common reconstruction path.
     *
     * 0x407c7c .. 0x407ca9
     */
    switch ((uint32_t)d) {

    case 0u:
        return data_in;

    case 1u:
        /*
         * 0x407d40:
         *   DataInReg << 8
         *   | PreviousReadReg byte at +3
         */
        return (data_in << 8) |
               ((previous_read >> 24) & 0x000000FFu);

    case 2u:
        /*
         * 0x407d60:
         *   DataInReg << 16
         *   | PreviousReadReg low 16 bits
         */
        return (data_in << 16) |
               (previous_read & 0x0000FFFFu);

    default:
        /*
         * 0x407c9b:
         *   DataInReg << 24
         *   | PreviousReadReg >> 8
         */
        return (data_in << 24) |
               (previous_read >> 8);
    }
}

static uint32_t select_xbus(
    uint32_t selector,
    t425_cpu_state_t *cpu,
    uint32_t constbox,
    uint32_t data_in,
    uint32_t channel_data,
    uint32_t *fptr0,
    uint32_t *fptr1)
{
    switch (selector) {
        case 0x200u: return cpu->IptrReg;
        case 0x100u: return constbox;
        case 0x080u: return cpu->Dreg;
        case 0x040u: return channel_data; /* ChannelData */
        case 0x020u: return data_in; /* DataIn */
        case 0x010u: return cpu->A;
        case 0x008u: return cpu->WptrReg;
        case 0x004u: return cpu->WdescReg;
        case 0x002u: return (fptr0 != NULL) ? *fptr0 : 0u; /* FPtrReg0 */
        case 0x001u: return (fptr1 != NULL) ? *fptr1 : 0u; /* FPtrReg1 */
        default: return 0u;
    }
}

static uint32_t select_ybus(
    uint32_t selector,
    t425_cpu_state_t *cpu,
    uint32_t constbox,
    uint32_t *bptr0,
    uint32_t *bptr1,
    uint32_t *clock_reg0,
    uint32_t *clock_reg1)
{
    switch (selector) {
        case 0x1000u: return cpu->Oreg << 2;
        case 0x0800u: return cpu->Oreg;
        case 0x0400u: return constbox;
        case 0x0200u: return cpu->Ereg;
        case 0x0100u: return cpu->B;
        case 0x0080u: return cpu->C << 1;
        case 0x0040u: return cpu->C;
        case 0x0020u: return (bptr0 != NULL) ? *bptr0 : 0u; /* BPtrReg0 */
        case 0x0010u: return (bptr1 != NULL) ? *bptr1 : 0u; /* BPtrReg1 */
        case 0x0008u: return cpu->StatusReg;
        case 0x0004u:
            return (clock_reg1 != NULL) ? *clock_reg1 : 0u; /* ClockReg1 */
        case 0x0002u:
            return (clock_reg0 != NULL) ? *clock_reg0 : 0u; /* ClockReg0 */
        case 0x0001u: return 0u; /* Byte select */
        default: return 0u;
    }
}

static uint32_t execute_alu(
    uint32_t alu,
    uint32_t x,
    uint32_t y,
    t425_datapath_t *datapath)
{
    uint32_t z;

    datapath->alu_carryout = 0u;
    datapath->alu_overflow = 0u;
    datapath->alu_prop_all1 = 0u;
    datapath->zbus_sub_less_than0 = 0u;

    switch (alu) {
        case 0x18u: /* X + Y */
        {
            uint64_t sum = (uint64_t)x + (uint64_t)y;

            z = (uint32_t)sum;

            datapath->alu_carryout =
                (uint8_t)((sum >> 32) & 1u);

            datapath->alu_overflow =
                (uint8_t)(
                    ((~(x ^ y) & (x ^ z)) >> 31) & 1u);

            break;
        }

        case 0x32u: /* X + 1 */
        {
            uint64_t sum = (uint64_t)x + 1u;

            z = (uint32_t)sum;

            datapath->alu_carryout =
                (uint8_t)((sum >> 32) & 1u);

            datapath->alu_overflow =
                (uint8_t)(
                    ((~(x ^ 1u) & (x ^ z)) >> 31) & 1u);

            break;
        }

        case 0x58u: /* Y - (X + 1) */
            z = y - (x + 1u);
            break;

        case 0x30u: /* X */
            z = x;
            break;

        case 0x70u: /* Xc */
            z = x;
            break;

        case 0x72u: /* -X */
            z = 0u - x;
            break;

        case 0x5Au: /* Y - X */
            z = y - x;
            break;

        case 0x1Au: /* Y + X + 1 */
        {
            uint64_t sum =
                (uint64_t)y +
                (uint64_t)x +
                1u;

            z = (uint32_t)sum;

            datapath->alu_carryout =
                (uint8_t)((sum >> 32) & 1u);

            datapath->alu_overflow =
                (uint8_t)(
                    ((~(x ^ y) & (x ^ z)) >> 31) & 1u);

            break;
        }

        case 0x26u: /* X - Y */
            z = x - y;
            break;

        case 0x66u: /* -Y */
            z = 0u - y;
            break;

        case 0x01u: /* 0 */
            z = 0u;
            break;

        case 0x21u: /* ~X & Y */
            z = (~x) & y;
            break;

        case 0x31u: /* ~X */
            z = ~x;
            break;

        case 0x09u: /* X & Y */
            z = x & y;
            break;

        case 0x29u: /* Y */
            z = y;
            break;

        case 0x69u: /* Yc */
            z = y;
            break;

        case 0x05u: /* ~X | Y */
            z = (~x) | y;
            break;

        case 0x25u: /* X xor Y */
            z = x ^ y;
            break;

        case 0x15u: /* ~Y */
            z = ~y;
            break;

        case 0x2Du: /* X | Y */
            z = x | y;
            break;

        case 0x3Du: /* -1 */
            z = 0xFFFFFFFFu;
            break;

        case 0x2Bu: /* Y & ~(4 - 1) */
            z = y & ~3u;
            break;

        case 0x0Du: /* X */
            z = x;
            break;

        default:
            z = 0u;
            break;
    }

    /*
     * T425 physical condition selector:
     *
     *   Cond0FromNotAluPropAll1
     *       is aliased by the VBC as
     *   Cond0FromNotZbusEq0.
     *
     * Therefore the propagation signal asserted here is the
     * zero-result state of the ALU:
     *
     *   AluPropAll1 = 1  when Zbus == 0
     *   AluPropAll1 = 0  when Zbus != 0
     *
     * and the externally selected condition becomes:
     *
     *   NotAluPropAll1 = 1 when Zbus != 0.
     */
    /*
     * T425 physical ALU condition tap recovered from t425emu_lite:
     *
     *     s_Minus             = Z[31]
     *     s_ZbusSubLessThan0  = !Z[31]
     *
     * The microsequencer consumes the complemented condition through
     * Cond0FromNotZbusSubLessThan0.
     */
    datapath->zbus_sub_less_than0 =
        (z & 0x80000000u) ? 1u : 0u;

    datapath->alu_prop_all1 = (z == 0u) ? 1u : 0u;

    return z;
}

/*
 * T425 A/B/C shift control.
 *
 * SHIFT field:
 *   bit 6 = ShiftRightAreg
 *   bit 5 = ShiftLeftBreg
 *   bit 4 = BregShIn0From1
 *   bit 3 = ShiftRightBreg
 *   bit 2 = CregFromZbusSR1
 *   bit 1 = CregShInMsbfromCarryOut
 *   bit 0 = CregShInMsbfromArithLogic
 *
 * All selected operations are parallel micro-operations.
 * Zbus is the result produced by the ALU during the same
 * microcycle.
 */
static void execute_shift_control(
    t425_cpu_state_t *cpu,
    uint32_t shiftsel,
    uint32_t zbus,
    uint8_t alu_carryout)
{
    uint32_t old_a = cpu->A;
    uint32_t old_b = cpu->B;
    uint32_t new_a = old_a;
    uint32_t new_b = old_b;
    uint32_t new_c = cpu->C;

    /*
     * ShiftRightAreg
     */
    if ((shiftsel & 0x40u) != 0u)
        new_a = old_a >> 1;

    /*
     * ShiftLeftBreg and BregShIn0From1 are independent
     * control bits belonging to the same shift-left path.
     */
    if ((shiftsel & 0x20u) != 0u)
        new_b = old_b << 1;

    if ((shiftsel & 0x10u) != 0u)
        new_b = (old_b << 1) | 1u;

    /*
     * ShiftRightBreg.
     */
    if ((shiftsel & 0x08u) != 0u)
        new_b = old_b >> 1;

    /*
     * CregFromZbusSR1.
     *
     * The base operation is C := Zbus >> 1.
     * The two MSB controls select the source inserted at C[31].
     */
    if ((shiftsel & 0x04u) != 0u) {
        new_c = zbus >> 1;

        if ((shiftsel & 0x02u) != 0u) {
            new_c |= ((uint32_t)(alu_carryout & 1u) << 31);
        } else if ((shiftsel & 0x01u) != 0u) {
            new_c |= (zbus & 0x80000000u);
        }
    }

    cpu->A = new_a;
    cpu->B = new_b;
    cpu->C = new_c;
}

static void write_zbus_destination(
    uint32_t selector,
    uint32_t z,
    t425_cpu_state_t *cpu,
    uint32_t *channel_select,
    uint32_t *channel_stack,
    uint32_t *data_mem_addr,
    uint32_t *fptr0,
    uint32_t *fptr1,
    uint32_t *bptr0,
    uint32_t *bptr1,
    uint32_t *clock_reg0,
    uint32_t *clock_reg1,
    uint32_t *tnext_reg0,
    uint32_t *tnext_reg1)
{
    /*
     * T425 VBC ZBUS is a 19-bit parallel destination mask.
     *
     * VBC field order:
     *   [103,98,79,77,69,65,57,51,49,47,45,43,41,38,36,34,12,8,0]
     *
     * Therefore multiple destinations may be written during the
     * same microcycle.  These are the definitive masks obtained
     * from the VBC field definition.
     */

    if ((selector & 0x40000u) != 0u) {
        cpu->Oreg = z;
    }

    if ((selector & 0x20000u) != 0u) {
        cpu->IptrReg = z;
    }

    if ((selector & 0x10000u) != 0u) {
        cpu->Ereg = z;
    }

    if ((selector & 0x08000u) != 0u) {
        cpu->Dreg = z;
    }

    if ((selector & 0x04000u) != 0u) {
        cpu->A = z;
    }

    if ((selector & 0x02000u) != 0u) {
        cpu->B = z;
    }

    if ((selector & 0x01000u) != 0u) {
        cpu->C = z;
    }

    if ((selector & 0x00800u) != 0u) {
        cpu->WdescReg = z;
    }

    if ((selector & 0x00040u) != 0u) {
        cpu->StatusReg = z;
    }

    /*
     * These three destinations have explicit storage in t425_core_t,
     * not in t425_cpu_state_t.
     */
    if ((selector & 0x00004u) != 0u && channel_select != NULL) {
        *channel_select = z;
    }

    if ((selector & 0x00002u) != 0u && channel_stack != NULL) {
        *channel_stack = z;
    }

    if ((selector & 0x00001u) != 0u && data_mem_addr != NULL) {
        *data_mem_addr = z;
    }

    /*
     * T425 VBC timer next-register destinations.
     *
     * Physical destinations:
     *   0x008 -> TNextReg0
     *   0x020 -> TNextReg1
     *
     * Existing architectural storage:
     *   TNextReg0 -> timer.next_high
     *   TNextReg1 -> timer.next_low
     */
    if ((selector & 0x00008u) != 0u && tnext_reg0 != NULL)
        *tnext_reg0 = z;

    if ((selector & 0x00020u) != 0u && tnext_reg1 != NULL)
        *tnext_reg1 = z;

    /*
     * T425 VBC ZBUS process-list destinations.
     *
     * Physical destinations:
     *
     *   0x400 -> BPtrReg0
     *   0x200 -> BPtrReg1
     *   0x100 -> FPtrReg0
     *   0x080 -> FPtrReg1
     *
     * These map onto the existing architectural scheduler
     * FPtr/BPtr storage.
     */
    if ((selector & 0x00010u) != 0u) {
        if (clock_reg0 != NULL)
            *clock_reg0 = z;
        if (clock_reg1 != NULL)
            *clock_reg1 = z;
    }

    if ((selector & 0x00400u) != 0u && bptr0 != NULL)
        *bptr0 = z;

    if ((selector & 0x00200u) != 0u && bptr1 != NULL)
        *bptr1 = z;

    if ((selector & 0x00100u) != 0u && fptr0 != NULL)
        *fptr0 = z;

    if ((selector & 0x00080u) != 0u && fptr1 != NULL)
        *fptr1 = z;
}

void t425_datapath_reset(t425_datapath_t *datapath)
{
    if (datapath == NULL) {
        return;
    }

    datapath->xbus = 0u;
    datapath->ybus = 0u;
    datapath->zbus = 0u;

    datapath->alu_carryout = 0u;
    datapath->alu_overflow = 0u;
    datapath->alu_prop_all1 = 0u;
    datapath->zbus_sub_less_than0 = 0u;

    datapath->constbox = 0u;
    datapath->byte_control = 0u;
    datapath->ybus_byte_select = 0u;

    t425_incrementor_reset(&datapath->incrementor);
}

void t425_datapath_execute(
    t425_datapath_t *datapath,
    t425_cpu_state_t *cpu,
    uint32_t *channel_select,
    uint32_t *channel_stack,
    uint32_t *data_mem_addr,
    uint32_t channel_data,
    uint32_t *fptr0,
    uint32_t *fptr1,
    uint32_t *bptr0,
    uint32_t *bptr1,
    uint32_t *clock_reg0,
    uint32_t *clock_reg1,
    uint32_t *tnext_reg0,
    uint32_t *tnext_reg1,
    const t425_mir_t *mir,
    uint32_t data_in,
    uint32_t previous_read_data,
    uint8_t byte_write_strobe_latch,
    t425_condition_signals_t *signals)
{
    uint32_t xsel;
    uint32_t ysel;
    uint32_t alusel;
    uint32_t zsel;
    uint32_t inc;
    uint32_t stacksel;
    uint32_t shiftsel;
    uint32_t old_a;
    uint32_t old_b;
    uint32_t old_c;

    if (datapath == NULL ||
        cpu == NULL ||
        mir == NULL ||
        signals == NULL) {
        return;
    }

    /*
     * T425 stack condition taps.
     *
     * These are combinational slave taps sampled from the register
     * state entering the microcycle. They must be captured before
     * any stack/shift destination is applied by this microword.
     */
    signals->a_slave0 = (uint8_t)(cpu->A & 1u);
    signals->b_slave0 = (uint8_t)(cpu->B & 1u);
    signals->b_slave1 = (uint8_t)((cpu->B >> 1) & 1u);
    signals->b_slave_msb = (uint8_t)((cpu->B >> 31) & 1u);
    signals->c_slave_msb = (uint8_t)((cpu->C >> 31) & 1u);

    /*
     * Stack transfers are parallel operations. Preserve the complete
     * pre-microcycle A/B/C state before applying any stack destination.
     */
    old_a = cpu->A;
    old_b = cpu->B;
    old_c = cpu->C;

    stacksel = decode_stack_control(mir);
    shiftsel = decode_shift_control(mir);

    xsel = decode_field(
        mir,
        xbus_bits,
        sizeof(xbus_bits) / sizeof(xbus_bits[0]));

    ysel = decode_field(
        mir,
        ybus_bits,
        sizeof(ybus_bits) / sizeof(ybus_bits[0]));

    alusel = decode_field(
        mir,
        alu_bits,
        sizeof(alu_bits) / sizeof(alu_bits[0]));

    zsel = decode_field(
        mir,
        zbus_bits,
        sizeof(zbus_bits) / sizeof(zbus_bits[0]));

    datapath->constbox = decode_constbox(mir);
    datapath->byte_control = t425_mir_byte(mir);

    /*
     * F20-09 — physical ByteUpShifter integration.
     *
     * DataIn and PreviousReadReg are temporal inputs.
     * A/B/C are the physical register inputs.
     *
     * The ByteWriteStrobeEnable latch is passed explicitly but its
     * architectural set/reset semantics are handled separately
     * in F20-10.
     */
    datapath->byte_upshifter_output =
        t425_byte_upshifter(
            cpu->A,
            cpu->B,
            cpu->C,
            data_in,
            previous_read_data,
            byte_write_strobe_latch);

    datapath->xbus =
        select_xbus(xsel, cpu, datapath->constbox, data_in,
                    channel_data,
                    fptr0,
                    fptr1);

    datapath->ybus =
        select_ybus(ysel, cpu, datapath->constbox,
                    bptr0,
                    bptr1,
                    clock_reg0,
                    clock_reg1);

    datapath->zbus =
        execute_alu(
            alusel,
            datapath->xbus,
            datapath->ybus,
            datapath);

    execute_shift_control(
        cpu,
        shiftsel,
        datapath->zbus,
        datapath->alu_carryout);

    write_zbus_destination(
        zsel,
        datapath->zbus,
        cpu,
        channel_select,
        channel_stack,
        data_mem_addr,
        fptr0,
        fptr1,
        bptr0,
        bptr1,
        clock_reg0,
        clock_reg1,
        tnext_reg0,
        tnext_reg1);

    /*
     * T425 Oreg-associated carry.
     *
     * Physical CROM[1B3] has INC=00 but performs:
     *
     *     Oreg <- All1s + Oreg
     *
     * followed immediately by:
     *
     *     Cond0FromNotIncOregCarryOut
     *
     * Therefore the carry associated with Oreg is established
     * by the ALU operation which writes Oreg, not exclusively
     * by WordIncOreg.
     */
    if ((zsel & 0x40000u) != 0u)
        datapath->incrementor.inc_oreg_carryout =
            datapath->alu_carryout;

    /*
     * T425 A/B/C stack control.
     *
     * The Slave sources are the register values captured at the
     * beginning of this microcycle, not values modified above.
     */
    /*
     * T425 A/B/C stack control is a parallel control field.
     * Multiple controls may be asserted in the same microword.
     * All Slave sources use the pre-microcycle snapshot.
     */
    if ((stacksel & 0x10u) != 0u) {       /* AregfromBregSlave */
        cpu->A = old_b;
    }

    if ((stacksel & 0x08u) != 0u) {       /* BregfromAregSlave */
        cpu->B = old_a;
    }

    if ((stacksel & 0x04u) != 0u) {       /* BregfromCregSlave */
        cpu->B = old_c;
    }

    if ((stacksel & 0x02u) != 0u) {       /* CregfromBregSlave */
        cpu->C = old_b;
    }

    if ((stacksel & 0x01u) != 0u) {       /* CregFrom0 */
        cpu->C = 0u;
    }

    inc = t425_mir_incrementor(mir);

    switch (inc) {
        case 0x8u:
            cpu->Oreg = 0u;
            datapath->incrementor.inc_oreg_carryout = 0u;
            datapath->incrementor.inc_dreg_carryout = 0u;
            break;

        case 0x4u:
            /*
             * OregFromZbusSL4.
             *
             * This is a destination operation from ZBUS,
             * shifted left four positions.
             */
            cpu->Oreg = datapath->zbus << 4;
            datapath->incrementor.inc_oreg_carryout = 0u;
            datapath->incrementor.inc_dreg_carryout = 0u;
            break;

        case 0x2u:
            t425_incrementor_word_oreg(
                &cpu->Oreg,
                &datapath->incrementor);
            break;

        case 0x1u:
            t425_incrementor_word_dreg(
                &cpu->Dreg,
                &datapath->incrementor);
            break;

        default:
            /*
             * INC=00 means no explicit increment operation.
             * Do not destroy an Oreg carry already established
             * by OregfromZbus in this microword.
             */
            datapath->incrementor.inc_dreg_carryout = 0u;
            break;
    }

    signals->not_inc_oreg_carryout =
        datapath->incrementor.inc_oreg_carryout ? 0u : 1u;

    signals->not_inc_dreg_carryout =
        datapath->incrementor.inc_dreg_carryout ? 0u : 1u;

    signals->not_carryout =
        datapath->alu_carryout ? 0u : 1u;

    signals->not_alu_prop_all1 =
        datapath->alu_prop_all1 ? 0u : 1u;

    signals->not_zbus_sub_less_than0 =
        datapath->zbus_sub_less_than0 ? 0u : 1u;

    signals->overflow =
        datapath->alu_overflow;
}
