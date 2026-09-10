#include <stdint.h>

#include "t425_core.h"
#include "t425_instruction.h"

static void push(t425_cpu_state_t *cpu, uint32_t value)
{
    cpu->C = cpu->B;
    cpu->B = cpu->A;
    cpu->A = value;
}

static void form_pfix(t425_cpu_state_t *cpu)
{
    cpu->Oreg <<= 4;
}

static void form_nfix(t425_cpu_state_t *cpu)
{
    cpu->Oreg = (~cpu->Oreg) << 4;
}

static uint32_t word_address(uint32_t base, uint32_t operand)
{
    return base + (operand << 2);
}

static void finish(t425_core_t *core, uint32_t cycles)
{
    core->cpu.Oreg = 0u;
    core->cycles += cycles;
    core->instructions++;
}

int t425_core_step(t425_core_t *core)
{
    t425_cpu_state_t *cpu;
    uint8_t instruction;
    uint8_t opcode;
    uint8_t nibble;
    uint32_t address;
    uint32_t value;

    if (!core)
        return T425_STEP_BADMEM;

    cpu = &core->cpu;

    if (cpu->halted)
        return T425_STEP_HALTED;

    if (t425_memory_read_byte(
            &core->memory, cpu->Iptr, &instruction) != 0)
        return T425_STEP_BADMEM;

    cpu->Iptr += 1u;

    opcode = (uint8_t)(instruction >> 4);
    nibble = (uint8_t)(instruction & 0x0Fu);

    cpu->Oreg = (cpu->Oreg & 0xFFFFFFF0u) | nibble;

    switch (opcode)
    {
        case T425_OP_PFIX:
            form_pfix(cpu);
            core->cycles += 1u;
            core->instructions++;
            return T425_STEP_OK;

        case T425_OP_NFIX:
            form_nfix(cpu);
            core->cycles += 1u;
            core->instructions++;
            return T425_STEP_OK;

        case T425_OP_LDLP:
            push(cpu, cpu->Wptr + (cpu->Oreg << 2));
            finish(core, 1u);
            return T425_STEP_OK;

        case T425_OP_LDNLP:
            cpu->A += (cpu->Oreg << 2);
            finish(core, 1u);
            return T425_STEP_OK;

        case T425_OP_LDL:
            address = word_address(cpu->Wptr, cpu->Oreg);

            if (t425_memory_read_word(
                    &core->memory, address, &value) != 0)
                return T425_STEP_BADMEM;

            push(cpu, value);
            finish(core, 2u);
            return T425_STEP_OK;

        case T425_OP_LDNL:
            address = word_address(cpu->A, cpu->Oreg);

            if (t425_memory_read_word(
                    &core->memory, address, &value) != 0)
                return T425_STEP_BADMEM;

            cpu->A = value;
            finish(core, 2u);
            return T425_STEP_OK;

        case T425_OP_AJW:
            cpu->Wptr += (uint32_t)((int32_t)cpu->Oreg * 4);
            finish(core, 1u);
            return T425_STEP_OK;

        case T425_OP_EQC:
            cpu->A = (cpu->A == cpu->Oreg) ? 1u : 0u;
            finish(core, 2u);
            return T425_STEP_OK;

        case T425_OP_STL:
            address = cpu->Wptr + (cpu->Oreg << 2);

            if (t425_memory_write_word(
                    &core->memory, address, cpu->A) != 0)
                return T425_STEP_BADMEM;

            cpu->A = cpu->B;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;

        case T425_OP_STNL:
            address = cpu->A + (cpu->Oreg << 2);

            if (t425_memory_write_word(
                    &core->memory, address, cpu->B) != 0)
                return T425_STEP_BADMEM;

            cpu->A = cpu->C;
            finish(core, 2u);
            return T425_STEP_OK;

        case T425_OP_ADC:
        {
            uint32_t old = cpu->A;
            uint32_t result = old + cpu->Oreg;

            if ((~(old ^ cpu->Oreg) & (old ^ result) &
                 0x80000000u) != 0u)
                cpu->error = 1u;

            cpu->A = result;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case T425_OP_LDC:
            push(cpu, cpu->Oreg);
            finish(core, 1u);
            return T425_STEP_OK;

        case T425_OP_J:
            cpu->Iptr += (uint32_t)(int32_t)cpu->Oreg;
            cpu->A = 0u;
            cpu->B = 0u;
            cpu->C = 0u;
            finish(core, 3u);
            return T425_STEP_OK;

        case T425_OP_CJ:
            if (cpu->A == 0u)
            {
                cpu->Iptr += (uint32_t)(int32_t)cpu->Oreg;
                finish(core, 4u);
            }
            else
            {
                cpu->A = cpu->B;
                cpu->B = cpu->C;
                finish(core, 2u);
            }
            return T425_STEP_OK;

        case T425_OP_CALL:
        {
            uint32_t old_wptr = cpu->Wptr;
            uint32_t old_iptr = cpu->Iptr;
            uint32_t old_a = cpu->A;
            uint32_t old_b = cpu->B;
            uint32_t old_c = cpu->C;
            uint32_t new_wptr = old_wptr - 16u;

            if (t425_memory_write_word(
                    &core->memory, new_wptr + 0u, old_iptr) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(
                    &core->memory, new_wptr + 4u, old_a) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(
                    &core->memory, new_wptr + 8u, old_b) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(
                    &core->memory, new_wptr + 12u, old_c) != 0)
                return T425_STEP_BADMEM;

            cpu->Wptr = new_wptr;
            cpu->A = old_iptr;
            cpu->Iptr = old_iptr + (uint32_t)(int32_t)cpu->Oreg;

            finish(core, 7u);
            return T425_STEP_OK;
        }

        case T425_OP_OPR:
            if (cpu->Oreg == 0x20u)
            {
                if (t425_memory_read_word(
                        &core->memory, cpu->Wptr, &value) != 0)
                    return T425_STEP_BADMEM;

                cpu->Iptr = value;
                cpu->Wptr += 16u;
                finish(core, 5u);
                return T425_STEP_OK;
            }

            return T425_STEP_UNIMPL;

        default:
            return T425_STEP_UNIMPL;
    }
}
