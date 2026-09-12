#include <stdint.h>

#include "t425_core.h"
#include "t425_process.h"
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


static int t425_enqueue(t425_core_t *core, uint32_t wdesc)
{
    t425_scheduler_state_t *scheduler = &core->scheduler;
    uint32_t wptr = t425_wdesc_wptr(wdesc);
    uint32_t *head;
    uint32_t *tail;

    if (t425_wdesc_priority(wdesc) == T425_PRIORITY_HIGH)
    {
        head = &scheduler->high_head;
        tail = &scheduler->high_tail;
    }
    else
    {
        head = &scheduler->low_head;
        tail = &scheduler->low_tail;
    }

    if (t425_memory_write_word(
            &core->memory, wptr - 8u, T425_NOT_PROCESS) != 0)
        return T425_STEP_BADMEM;

    if (*head == T425_NOT_PROCESS)
    {
        *head = wdesc;
        *tail = wdesc;
        return T425_STEP_OK;
    }

    if (t425_memory_write_word(
            &core->memory,
            t425_wdesc_wptr(*tail) - 8u,
            wdesc) != 0)
        return T425_STEP_BADMEM;

    *tail = wdesc;
    return T425_STEP_OK;
}

static int t425_schedule_next(t425_core_t *core)
{
    t425_scheduler_state_t *scheduler = &core->scheduler;
    uint32_t next_wdesc;
    uint32_t next_wptr;
    uint32_t next_iptr;
    uint32_t next_link;

    if (scheduler->high_head != T425_NOT_PROCESS)
        next_wdesc = scheduler->high_head;
    else if (scheduler->low_head != T425_NOT_PROCESS)
        next_wdesc = scheduler->low_head;
    else
    {
        scheduler->current_wdesc = T425_NOT_PROCESS;
        scheduler->running = 0u;
        return T425_STEP_OK;
    }

    next_wptr = t425_wdesc_wptr(next_wdesc);

    if (t425_memory_read_word(
            &core->memory, next_wptr - 4u, &next_iptr) != 0)
        return T425_STEP_BADMEM;

    if (t425_memory_read_word(
            &core->memory, next_wptr - 8u, &next_link) != 0)
        return T425_STEP_BADMEM;

    if (t425_wdesc_priority(next_wdesc) == T425_PRIORITY_HIGH)
    {
        scheduler->high_head = next_link;

        if (next_link == T425_NOT_PROCESS)
            scheduler->high_tail = T425_NOT_PROCESS;
    }
    else
    {
        scheduler->low_head = next_link;

        if (next_link == T425_NOT_PROCESS)
            scheduler->low_tail = T425_NOT_PROCESS;
    }

    core->cpu.Wptr = next_wptr;
    core->cpu.Iptr = next_iptr;

    scheduler->current_wdesc = next_wdesc;
    scheduler->running = 1u;

    return T425_STEP_OK;
}

static int t425_opr_execute(t425_core_t *core)
{
    t425_cpu_state_t *cpu = &core->cpu;
    uint32_t value;

    switch (cpu->Oreg)
    {
        case 0x55u: /* STOPERR */
            if (!cpu->error)
            {
                finish(core, 2u);
                return T425_STEP_OK;
            }

            if (t425_schedule_next(core) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            finish(core, 2u);
            return T425_STEP_OK;

        case 0x0Du: /* STARTP */
        {
            uint32_t new_wptr = cpu->A;
            uint32_t new_iptr = cpu->Iptr + cpu->B;
            uint32_t new_wdesc;

            if ((new_wptr & 3u) != 0u)
                return T425_STEP_BADMEM;

            if (core->scheduler.current_wdesc == T425_NOT_PROCESS)
                return T425_STEP_BADMEM;

            new_wdesc =
                new_wptr |
                t425_wdesc_priority(core->scheduler.current_wdesc);

            if (t425_memory_write_word(
                    &core->memory,
                    new_wptr - 4u,
                    new_iptr) != 0)
                return T425_STEP_BADMEM;

            if (t425_enqueue(core, new_wdesc) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            cpu->A = 0u;
            cpu->B = 0u;
            cpu->C = 0u;

            finish(core, 12u);
            return T425_STEP_OK;
        }

case 0x03u: /* ENDP */
        {
            uint32_t successor_wptr = cpu->A;
            uint32_t count;

            if (t425_memory_read_word(
                    &core->memory,
                    successor_wptr + 4u,
                    &count) != 0)
                return T425_STEP_BADMEM;

            if (count == 0u)
                return T425_STEP_BADMEM;

            if (count == 1u)
            {
                uint32_t successor_iptr;

                if (t425_memory_read_word(
                        &core->memory,
                        successor_wptr,
                        &successor_iptr) != 0)
                    return T425_STEP_BADMEM;

                cpu->Wptr = successor_wptr;
                cpu->Iptr = successor_iptr;
                core->scheduler.current_wdesc =
                    successor_wptr |
                    t425_wdesc_priority(core->scheduler.current_wdesc);
                core->scheduler.running = 1u;

                cpu->A = 0u;
                cpu->B = 0u;
                cpu->C = 0u;

                finish(core, 13u);
                return T425_STEP_OK;
            }

            if (t425_memory_write_word(
                    &core->memory,
                    successor_wptr + 4u,
                    count - 1u) != 0)
                return T425_STEP_BADMEM;

            if (t425_schedule_next(core) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            finish(core, 13u);
            return T425_STEP_OK;
        }

        case 0x39u: /* RUNP */
        {
            uint32_t wdesc = cpu->A;
            uint32_t wptr = t425_wdesc_wptr(wdesc);
            uint32_t iptr;

            if ((wptr & 3u) != 0u ||
                wdesc == T425_NOT_PROCESS)
                return T425_STEP_BADMEM;

            if (t425_memory_read_word(
                    &core->memory,
                    wptr - 4u,
                    &iptr) != 0)
                return T425_STEP_BADMEM;

            if (t425_enqueue(core, wdesc) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            cpu->A = 0u;
            finish(core, 10u);
            return T425_STEP_OK;
        }

        case 0x15u: /* STOPP */
        {
            if (core->scheduler.current_wdesc == T425_NOT_PROCESS)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(
                    &core->memory,
                    cpu->Wptr - 4u,
                    cpu->Iptr) != 0)
                return T425_STEP_BADMEM;

            if (t425_schedule_next(core) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            cpu->A = 0u;
            cpu->B = 0u;
            cpu->C = 0u;

            finish(core, 11u);
            return T425_STEP_OK;
        }

        case 0x1Eu: /* LDPRI */
            push(cpu,
                 t425_wdesc_priority(core->scheduler.current_wdesc));
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x10u: /* SETERR */
    cpu->error = 1u;
    finish(core, 1u);
    return T425_STEP_OK;

  case 0x29u: /* TESTERR */
    if (cpu->error) {
        cpu->error = 0u;
        push(cpu, 1u);
        finish(core, 3u);
    } else {
        push(cpu, 0u);
        finish(core, 2u);
    }
    return T425_STEP_OK;

  case 0x57u: /* CLRHALTERR */
            cpu->halt_on_error = 0u;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x58u: /* SETHALTERR */
            cpu->halt_on_error = 1u;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x59u: /* TESTHALTERR */
            push(cpu, cpu->halt_on_error ? 1u : 0u);
            finish(core, 2u);
            return T425_STEP_OK;

  case 0x20u: /* RET */
            if (t425_memory_read_word(
                    &core->memory, cpu->Wptr, &value) != 0)
                return T425_STEP_BADMEM;

            cpu->Iptr = value;
            cpu->Wptr += 16u;
            finish(core, 5u);
            return T425_STEP_OK;

        case 0x46u: /* AND */
            cpu->A = cpu->A & cpu->B;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x4Bu: /* OR */
            cpu->A = cpu->A | cpu->B;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x33u: /* XOR */
            cpu->A = cpu->A ^ cpu->B;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x32u: /* NOT */
            cpu->A = ~cpu->A;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x05u: /* ADD */
        {
            uint32_t old_a = cpu->A;
            uint32_t old_b = cpu->B;
            uint32_t result = old_a + old_b;

            if ((~(old_a ^ old_b) & (old_a ^ result) &
                 0x80000000u) != 0u)
                cpu->error = 1u;

            cpu->A = result;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x0Cu: /* SUB */
        {
            uint32_t old_a = cpu->A;
            uint32_t old_b = cpu->B;
            uint32_t result = old_b - old_a;

            if (((old_b ^ old_a) & (old_b ^ result) &
                 0x80000000u) != 0u)
                cpu->error = 1u;

            cpu->A = result;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x04u: /* DIFF */
            cpu->A = cpu->B - cpu->A;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x52u: /* SUM */
            cpu->A = cpu->B + cpu->A;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;

        case 0x09u: /* GT */
        {
            uint32_t old_a = cpu->A;
            uint32_t old_b = cpu->B;

            cpu->A = (old_b > old_a) ? 1u : 0u;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x53u: /* MUL */
            cpu->A = cpu->A * cpu->B;
            cpu->B = cpu->C;
            finish(core, 38u);
            return T425_STEP_OK;

        case 0x08u: /* PROD */
        {
            uint32_t original_a = cpu->A;
            uint32_t magnitude;
            uint32_t bit = 0u;
            uint32_t cycles;

            if ((original_a & 0x80000000u) != 0u)
                magnitude = 0u - original_a;
            else
                magnitude = original_a;

            if (magnitude != 0u)
            {
                uint32_t value = magnitude;
                while (value > 1u)
                {
                    value >>= 1;
                    bit++;
                }
            }

            cpu->A = original_a * cpu->B;
            cpu->B = cpu->C;

            if ((original_a & 0x80000000u) != 0u)
                cycles = bit + 5u;
            else
                cycles = bit + 4u;

            finish(core, cycles);
            return T425_STEP_OK;
        }

        case 0x16u: /* LADD */
        {
            uint64_t result =
                (uint64_t)cpu->B +
                (uint64_t)cpu->A +
                (uint64_t)(cpu->C & 1u);

            if (result > 0xFFFFFFFFu)
                cpu->error = 1u;

            cpu->A = (uint32_t)result;
            cpu->B = cpu->C;
            finish(core, 2u);
            return T425_STEP_OK;
        }

        case 0x38u: /* LSUB */
        {
            uint64_t subtrahend =
                (uint64_t)cpu->A +
                (uint64_t)(cpu->C & 1u);

            uint32_t old_b = cpu->B;

            if ((uint64_t)old_b < subtrahend)
                cpu->error = 1u;

            cpu->A = (uint32_t)((uint64_t)old_b - subtrahend);
            cpu->B = cpu->C;
            finish(core, 2u);
            return T425_STEP_OK;
        }

        case 0x37u: /* LSUM */
        {
            uint64_t result =
                (uint64_t)cpu->B +
                (uint64_t)cpu->A +
                (uint64_t)(cpu->C & 1u);

            cpu->A = (uint32_t)result;
            cpu->B = (uint32_t)(result >> 32);
            finish(core, 3u);
            return T425_STEP_OK;
        }

        case 0x4Fu: /* LDIFF */
        {
            uint64_t subtrahend =
                (uint64_t)cpu->A +
                (uint64_t)(cpu->C & 1u);

            uint32_t old_b = cpu->B;
            uint32_t borrow =
                ((uint64_t)old_b < subtrahend) ? 1u : 0u;

            cpu->A = (uint32_t)((uint64_t)old_b - subtrahend);
            cpu->B = borrow;
            finish(core, 3u);
            return T425_STEP_OK;
        }

        case 0x2Cu: /* DIV */
        {
            int32_t divisor = (int32_t)cpu->A;
            int32_t dividend = (int32_t)cpu->B;

            if (divisor == 0)
            {
                cpu->error = 1u;
                cpu->B = cpu->C;
                finish(core, 39u);
                return T425_STEP_OK;
            }

            if (divisor == -1 && dividend == INT32_MIN)
            {
                cpu->error = 1u;
                cpu->B = cpu->C;
                finish(core, 39u);
                return T425_STEP_OK;
            }

            cpu->A = (uint32_t)(dividend / divisor);
            cpu->B = cpu->C;
            finish(core, 39u);
            return T425_STEP_OK;
        }

        case 0x1Fu: /* REM */
        {
            int32_t divisor = (int32_t)cpu->A;
            int32_t dividend = (int32_t)cpu->B;

            if (divisor == 0)
            {
                cpu->error = 1u;
                cpu->B = cpu->C;
                finish(core, 37u);
                return T425_STEP_OK;
            }

            if (divisor == -1 && dividend == INT32_MIN)
            {
                cpu->error = 1u;
                cpu->B = cpu->C;
                finish(core, 37u);
                return T425_STEP_OK;
            }

            cpu->A = (uint32_t)(dividend % divisor);
            cpu->B = cpu->C;
            finish(core, 37u);
            return T425_STEP_OK;
        }

        case 0x41u: /* SHL */
        {
            uint32_t n = cpu->A;
            uint32_t value = cpu->B;

            cpu->A = (n < 32u) ? (value << n) : 0u;
            cpu->B = cpu->C;
            finish(core, n + 2u);
            return T425_STEP_OK;
        }

        case 0x40u: /* SHR */
        {
            uint32_t n = cpu->A;
            uint32_t value = cpu->B;

            cpu->A = (n < 32u) ? (value >> n) : 0u;
            cpu->B = cpu->C;
            finish(core, n + 2u);
            return T425_STEP_OK;
        }

        case 0x31u: /* LMUL */
        {
            uint64_t product =
                (uint64_t)cpu->A * (uint64_t)cpu->B;

            product += (uint64_t)cpu->C;

            cpu->A = (uint32_t)product;
            cpu->B = (uint32_t)(product >> 32);

            finish(core, 33u);
            return T425_STEP_OK;
        }

        case 0x1Au: /* LDIV */
        {
            uint32_t divisor = cpu->A;
            uint64_t dividend =
                ((uint64_t)cpu->C << 32) | (uint64_t)cpu->B;

            if (divisor == 0u || cpu->C >= divisor) {
                cpu->error = 1u;
                finish(core, 35u);
                return T425_STEP_OK;
            }

            cpu->A = (uint32_t)(dividend / divisor);
            cpu->B = (uint32_t)(dividend % divisor);

            finish(core, 35u);
            return T425_STEP_OK;
        }

        case 0x36u: /* LSHL */
        {
            uint32_t n = cpu->A;
            uint64_t value =
                ((uint64_t)cpu->C << 32) | (uint64_t)cpu->B;
            uint64_t result =
                (n < 64u) ? (value << n) : 0u;
            uint32_t cycles =
                (n < 32u) ? (n + 3u) : (n - 28u);

            cpu->A = (uint32_t)result;
            cpu->B = (uint32_t)(result >> 32);

            finish(core, cycles);
            return T425_STEP_OK;
        }

        case 0x35u: /* LSHR */
        {
            uint32_t n = cpu->A;
            uint64_t value =
                ((uint64_t)cpu->C << 32) | (uint64_t)cpu->B;
            uint64_t result =
                (n < 64u) ? (value >> n) : 0u;
            uint32_t cycles =
                (n < 32u) ? (n + 3u) : (n - 28u);

            cpu->A = (uint32_t)result;
            cpu->B = (uint32_t)(result >> 32);

            finish(core, cycles);
            return T425_STEP_OK;
        }

        default:
            return T425_STEP_UNIMPL;
    }
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
            return t425_opr_execute(core);

        default:
            return T425_STEP_UNIMPL;
    }
}
