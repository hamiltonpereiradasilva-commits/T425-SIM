#include <stdint.h>

#include "t425_core.h"
#include "t425_process.h"
#include "t425_instruction.h"
#include "t425_timer.h"

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

static int t425_enqueue(t425_core_t *core, uint32_t wdesc);

static unsigned int t425_timer_alt_index(uint32_t wptr)
{
    return (unsigned int)((wptr >> 2) & 1023u);
}

static void t425_timer_alt_mark(t425_timer_state_t *timer, uint32_t wptr)
{
    unsigned int index = t425_timer_alt_index(wptr);
    timer->alt_wait_mask[index >> 5] |= 1u << (index & 31u);
}

static void t425_timer_alt_clear(t425_timer_state_t *timer, uint32_t wptr)
{
    unsigned int index = t425_timer_alt_index(wptr);
    timer->alt_wait_mask[index >> 5] &= ~(1u << (index & 31u));
}

static int t425_timer_alt_is_marked(
    const t425_timer_state_t *timer,
    uint32_t wptr)
{
    unsigned int index = t425_timer_alt_index(wptr);
    return (timer->alt_wait_mask[index >> 5] &
            (1u << (index & 31u))) != 0u;
}

static int t425_timer_service(t425_core_t *core)
{
    uint32_t head_addr;
    uint32_t head;
    uint32_t now;
    uint32_t next;
    uint32_t next_time;
    uint32_t wptr;
    int is_alt_wait;

    for (unsigned int pass = 0u; pass < 2u; ++pass)
    {
        if (pass == 0u)
        {
            head_addr = T425_TPTR_LOC0;
            now = t425_timer_read_high(&core->timer);
        }
        else
        {
            head_addr = T425_TPTR_LOC1;
            now = t425_timer_read_low(&core->timer);
        }

        for (;;)
        {
            if (t425_memory_read_word(&core->memory, head_addr, &head) != 0)
                return T425_STEP_BADMEM;

            if (head == T425_NOT_PROCESS)
                break;

            wptr = t425_wdesc_wptr(head);

            if (t425_memory_read_word(&core->memory,
                                      wptr - 20u,
                                      &next_time) != 0)
                return T425_STEP_BADMEM;

            if ((int32_t)(now - next_time) < 0)
                break;

            if (t425_memory_read_word(&core->memory,
                                      wptr - 16u,
                                      &next) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(&core->memory,
                                       head_addr,
                                       next) != 0)
                return T425_STEP_BADMEM;

            if (next == T425_NOT_PROCESS)
            {
                if (pass == 0u)
                    core->timer.next_high = UINT32_MAX;
                else
                    core->timer.next_low = UINT32_MAX;
            }
            else
            {
                if (t425_memory_read_word(&core->memory,
                                          t425_wdesc_wptr(next) - 20u,
                                          &next_time) != 0)
                    return T425_STEP_BADMEM;

                if (pass == 0u)
                    core->timer.next_high = next_time;
                else
                    core->timer.next_low = next_time;
            }

            is_alt_wait = t425_timer_alt_is_marked(&core->timer, wptr);
            t425_timer_alt_clear(&core->timer, wptr);

            if (t425_memory_write_word(
                    &core->memory,
                    wptr - 16u,
                    is_alt_wait ? T425_TIMER_TIME_SET
                                : T425_NOT_PROCESS) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(&core->memory,
                                       wptr - 12u,
                                       T425_PROCESS_STATE_READY) != 0)
                return T425_STEP_BADMEM;

            if (t425_enqueue(core, head) != T425_STEP_OK)
                return T425_STEP_BADMEM;
        }
    }

    return T425_STEP_OK;
}

static void finish(t425_core_t *core, uint32_t cycles)
{
    core->cpu.Oreg = 0u;
    t425_timer_advance(&core->timer, cycles);
    core->cycles += cycles;
    core->instructions++;
    (void)t425_timer_service(core);
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

static int t425_timer_enqueue(
    t425_core_t *core,
    uint32_t wdesc,
    uint32_t time,
    int alt_wait)
{
    uint32_t head_addr;
    uint32_t head;
    uint32_t current;
    uint32_t previous;
    uint32_t current_time;
    uint32_t next;
    uint32_t wptr = t425_wdesc_wptr(wdesc);
    uint32_t *next_reg;

    if (t425_wdesc_priority(wdesc) == T425_PRIORITY_HIGH)
    {
        head_addr = T425_TPTR_LOC0;
        next_reg = &core->timer.next_high;
    }
    else
    {
        head_addr = T425_TPTR_LOC1;
        next_reg = &core->timer.next_low;
    }

    if (t425_memory_read_word(&core->memory, head_addr, &head) != 0)
        return T425_STEP_BADMEM;

    if (t425_memory_write_word(&core->memory, wptr - 20u, time) != 0 ||
        t425_memory_write_word(&core->memory, wptr - 12u,
                               T425_PROCESS_STATE_WAITING) != 0)
        return T425_STEP_BADMEM;

    previous = T425_NOT_PROCESS;
    current = head;

    while (current != T425_NOT_PROCESS)
    {
        uint32_t current_wptr = t425_wdesc_wptr(current);
        if (t425_memory_read_word(&core->memory,
                                  current_wptr - 20u,
                                  &current_time) != 0)
            return T425_STEP_BADMEM;

        if ((int32_t)(time - current_time) < 0)
            break;

        previous = current;
        if (t425_memory_read_word(&core->memory,
                                  current_wptr - 16u,
                                  &current) != 0)
            return T425_STEP_BADMEM;
    }

    next = current;

    if (t425_memory_write_word(&core->memory, wptr - 16u, next) != 0)
        return T425_STEP_BADMEM;

    if (previous == T425_NOT_PROCESS)
    {
        if (t425_memory_write_word(&core->memory, head_addr, wdesc) != 0)
            return T425_STEP_BADMEM;
        *next_reg = time;
    }
    else
    {
        uint32_t previous_wptr = t425_wdesc_wptr(previous);
        if (t425_memory_write_word(&core->memory,
                                   previous_wptr - 16u,
                                   wdesc) != 0)
            return T425_STEP_BADMEM;
    }

    if (alt_wait)
        t425_timer_alt_mark(&core->timer, wptr);

    return T425_STEP_OK;
}

static int t425_timer_remove(
    t425_core_t *core,
    uint32_t wdesc)
{
    uint32_t head_addr;
    uint32_t head;
    uint32_t current;
    uint32_t previous;
    uint32_t next;
    uint32_t current_wptr;
    uint32_t previous_wptr;

    if (t425_wdesc_priority(wdesc) == T425_PRIORITY_HIGH)
        head_addr = T425_TPTR_LOC0;
    else
        head_addr = T425_TPTR_LOC1;

    if (t425_memory_read_word(&core->memory, head_addr, &head) != 0)
        return T425_STEP_BADMEM;

    previous = T425_NOT_PROCESS;
    current = head;

    while (current != T425_NOT_PROCESS)
    {
        if (current == wdesc)
        {
            current_wptr = t425_wdesc_wptr(current);

            if (t425_memory_read_word(&core->memory,
                                      current_wptr - 16u,
                                      &next) != 0)
                return T425_STEP_BADMEM;

            if (previous == T425_NOT_PROCESS)
            {
                if (t425_memory_write_word(&core->memory,
                                           head_addr,
                                           next) != 0)
                    return T425_STEP_BADMEM;
            }
            else
            {
                previous_wptr = t425_wdesc_wptr(previous);

                if (t425_memory_write_word(&core->memory,
                                           previous_wptr - 16u,
                                           next) != 0)
                    return T425_STEP_BADMEM;
            }

            if (next == T425_NOT_PROCESS)
            {
                if (t425_wdesc_priority(wdesc) == T425_PRIORITY_HIGH)
                    core->timer.next_high = UINT32_MAX;
                else
                    core->timer.next_low = UINT32_MAX;
            }
            else
            {
                uint32_t next_time;

                if (t425_memory_read_word(
                        &core->memory,
                        t425_wdesc_wptr(next) - 20u,
                        &next_time) != 0)
                    return T425_STEP_BADMEM;

                if (t425_wdesc_priority(wdesc) == T425_PRIORITY_HIGH)
                    core->timer.next_high = next_time;
                else
                    core->timer.next_low = next_time;
            }

            t425_timer_alt_clear(&core->timer, wdesc);
            return T425_STEP_OK;
        }

        previous = current;

        current_wptr = t425_wdesc_wptr(current);

        if (t425_memory_read_word(&core->memory,
                                  current_wptr - 16u,
                                  &current) != 0)
            return T425_STEP_BADMEM;
    }

    /*
     * The process may already have timed out and been removed.
     * In that case there is nothing left to unlink.
     */
    t425_timer_alt_clear(&core->timer, wdesc);
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

        case 0x13u: /* CSUB0 */
            if (cpu->B > cpu->A)
                cpu->error = 1u;

            cpu->A = cpu->B;
            cpu->B = cpu->C;
            finish(core, 2u);
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

        case 0x00u: /* REV */
        {
            uint32_t value = cpu->A;
            cpu->A = cpu->B;
            cpu->B = value;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x3Au: /* XWORD */
        {
            uint32_t sign_bit = cpu->A;
            uint32_t value = cpu->B;
            uint32_t mask;
            uint32_t extended;

            /*
             * A contains 2^(N-1), identifying the sign bit of
             * the N-bit signed object.  XWORD sign-extends B.
             * The result occupies A and the old C is promoted to B.
             */
            if (sign_bit == 0u ||
                (sign_bit & (sign_bit - 1u)) != 0u)
            {
                cpu->error = 1u;
                finish(core, 4u);
                return T425_STEP_OK;
            }

            mask = (sign_bit << 1u) - 1u;
            extended = value & mask;

            if ((value & sign_bit) != 0u)
                extended |= ~mask;

            cpu->A = extended;
            cpu->B = cpu->C;
            finish(core, 4u);
            return T425_STEP_OK;
        }

        case 0x56u: /* CWORD */
        {
            uint32_t sign_bit = cpu->A;

            /*
             * A contains 2^(N-1).  A signed N-bit value is in
             * [-A, A-1].
             */
            if (sign_bit == 0u ||
                (sign_bit & (sign_bit - 1u)) != 0u)
            {
                cpu->error = 1u;
            }
            else
            {
                int64_t value = (int64_t)(int32_t)cpu->B;
                int64_t minimum = -(int64_t)sign_bit;
                int64_t maximum = (int64_t)sign_bit - 1;

                if (value < minimum || value > maximum)
                    cpu->error = 1u;
            }

            cpu->A = cpu->B;
            cpu->B = cpu->C;
            finish(core, 5u);
            return T425_STEP_OK;
        }

        case 0x1Du: /* XDBLE */
        {
            uint32_t value = cpu->A;

            cpu->C = cpu->B;
            cpu->B = (value & 0x80000000u) != 0u
                         ? UINT32_MAX
                         : 0u;

            finish(core, 2u);
            return T425_STEP_OK;
        }

        case 0x4Cu: /* CSNGL */
        {
            uint32_t high = cpu->B;
            uint32_t sign = (cpu->A & 0x80000000u) != 0u;

            /*
             * A/B is a signed 64-bit integer, with B as MSW.
             * The value is representable as a signed word iff B
             * is either all zeroes or all ones matching A's sign.
             */
            if ((!sign && high != 0u) ||
                (sign && high != UINT32_MAX))
            {
                cpu->error = 1u;
            }

            cpu->B = cpu->C;
            finish(core, 3u);
            return T425_STEP_OK;
        }

        case 0x42u: /* MINT */
        {
            cpu->A = 0x80000000u;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x5Au: /* DUP */
        {
            uint32_t old_a = cpu->A;
            uint32_t old_b = cpu->B;

            cpu->C = old_b;
            cpu->B = old_a;
            /* A remains unchanged. */

            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x79u: /* POP */
        {
            uint32_t old_a = cpu->A;
            uint32_t old_b = cpu->B;
            uint32_t old_c = cpu->C;

            cpu->A = old_b;
            cpu->B = old_c;
            cpu->C = old_a;

            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x01u: /* LB */
        {
            uint8_t value;

            if (t425_memory_read_byte(&core->memory, cpu->A, &value) != 0)
                return T425_STEP_BADMEM;

            cpu->A = (uint32_t)value;
            finish(core, 5u);
            return T425_STEP_OK;
        }

        case 0x02u: /* BSUB */
        {
            cpu->A += cpu->B;
            cpu->B = cpu->C;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x0Au: /* WSUB */
        {
            cpu->A += cpu->B * 4u;
            cpu->B = cpu->C;
            finish(core, 2u);
            return T425_STEP_OK;
        }

        case 0x17u: /* STLB */
        {
            core->scheduler.low_tail = cpu->A;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x18u: /* STHF */
        {
            core->scheduler.high_head = cpu->A;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x1Cu: /* STLF */
        {
            core->scheduler.low_head = cpu->A;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x3Bu: /* SB */
        {
            if (t425_memory_write_byte(&core->memory,
                                       cpu->A,
                                       (uint8_t)(cpu->B & 0xFFu)) != 0)
                return T425_STEP_BADMEM;

            cpu->A = cpu->C;
            finish(core, 4u);
            return T425_STEP_OK;
        }

        case 0x34u: /* BCNT */
        {
            cpu->A *= 4u;
            finish(core, 2u);
            return T425_STEP_OK;
        }

        case 0x3Fu: /* WCNT */
        {
            uint32_t address = cpu->A;
            uint32_t old_b = cpu->B;

            cpu->A = (address & ~3u) / 4u;
            cpu->B = address & 3u;
            cpu->C = old_b;

            finish(core, 5u);
            return T425_STEP_OK;
        }

        case 0x50u: /* STHB */
        {
            core->scheduler.high_tail = cpu->A;
            finish(core, 1u);
            return T425_STEP_OK;
        }

        case 0x81u: /* WSUBDB */
        {
            cpu->A += cpu->B * 8u;
            cpu->B = cpu->C;
            finish(core, 3u);
            return T425_STEP_OK;
        }

        case 0x19u: /* NORM */
        {
            uint32_t high = cpu->B;
            uint32_t low = cpu->A;
            uint32_t shifts = 0u;

            /*
             * Normalize the unsigned double-word B:A.
             * B is the most significant word.
             *
             * T425 semantics:
             *   shift left until B bit 31 is set;
             *   C receives the number of shifts;
             *   zero input produces C == 64.
             */
            while ((high & 0x80000000u) == 0u && shifts < 64u)
            {
                high = (high << 1u) | (low >> 31u);
                low <<= 1u;
                ++shifts;
            }

            cpu->A = low;
            cpu->B = high;
            cpu->C = shifts;

            if (shifts == 64u)
                finish(core, 3u);
            else if (shifts < 32u)
                finish(core, shifts + 5u);
            else
                finish(core, shifts - 26u);

            return T425_STEP_OK;
        }

        case 0x21u: /* LEND */
        {
            uint32_t index;
            uint32_t count;

            if (t425_memory_read_word(&core->memory,
                                      cpu->B,
                                      &index) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_read_word(&core->memory,
                                      cpu->B + 4u,
                                      &count) != 0)
                return T425_STEP_BADMEM;

            count -= 1u;

            if (t425_memory_write_word(&core->memory,
                                       cpu->B + 4u,
                                       count) != 0)
                return T425_STEP_BADMEM;

            if (count != 0u)
            {
                index += 1u;

                if (t425_memory_write_word(&core->memory,
                                           cpu->B,
                                           index) != 0)
                    return T425_STEP_BADMEM;

                cpu->Iptr -= cpu->A;
                finish(core, 10u);
            }
            else
            {
                finish(core, 5u);
            }

            return T425_STEP_OK;
        }

        case 0x3Cu: /* GAJW */
        {
            uint32_t old_a = cpu->A;
            uint32_t old_wptr = cpu->Wptr;

            cpu->A = old_wptr;
            cpu->Wptr = old_a & ~3u;

            finish(core, 2u);
            return T425_STEP_OK;
        }

        case 0x4Au: /* MOVE */
        {
            uint32_t count = cpu->A;
            uint32_t source = cpu->C;
            uint32_t destination = cpu->B;
            uint32_t i;

            /*
             * Architectural MOVE copies A bytes from C to B.
             * Overlapping source/destination regions have undefined
             * architectural results; this implementation performs the
             * natural forward byte copy.
             */
            for (i = 0u; i < count; ++i)
            {
                uint8_t value;

                if (t425_memory_read_byte(&core->memory,
                                          source + i,
                                          &value) != 0)
                    return T425_STEP_BADMEM;

                if (t425_memory_write_byte(&core->memory,
                                           destination + i,
                                           value) != 0)
                    return T425_STEP_BADMEM;
            }

            finish(core, 2u * ((count + 3u) / 4u) + 8u);
            return T425_STEP_OK;
        }

        case 0x4Du: /* CCNT1 */
        {
            uint32_t count = cpu->B;

            if (count == 0u || count > cpu->A)
                cpu->error = 1u;

            cpu->A = count;
            cpu->B = cpu->C;

            finish(core, 3u);
            return T425_STEP_OK;
        }

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

        case 0x4Eu: /* TALT */
        {
            if (t425_memory_write_word(&core->memory,
                                       cpu->Wptr - 12u,
                                       T425_PROCESS_STATE_ENABLING) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(&core->memory,
                                       cpu->Wptr - 16u,
                                       T425_TIMER_TIME_NO_SET) != 0)
                return T425_STEP_BADMEM;

            finish(core, 4u);
            return T425_STEP_OK;
        }

        case 0x47u: /* ENBT */
        {
            uint32_t tlink;
            uint32_t time;

            if (cpu->A == 0u)
            {
                finish(core, 8u);
                return T425_STEP_OK;
            }

            if (t425_memory_read_word(&core->memory,
                                      cpu->Wptr - 16u,
                                      &tlink) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_read_word(&core->memory,
                                      cpu->Wptr - 20u,
                                      &time) != 0)
                return T425_STEP_BADMEM;

            if (tlink == T425_TIMER_TIME_NO_SET)
            {
                if (t425_memory_write_word(&core->memory,
                                           cpu->Wptr - 20u,
                                           cpu->B) != 0)
                    return T425_STEP_BADMEM;

                if (t425_memory_write_word(&core->memory,
                                           cpu->Wptr - 16u,
                                           T425_TIMER_TIME_SET) != 0)
                    return T425_STEP_BADMEM;
            }
            else if (tlink == T425_TIMER_TIME_SET)
            {
                if ((int32_t)(time - cpu->B) > 0)
                {
                    if (t425_memory_write_word(&core->memory,
                                               cpu->Wptr - 20u,
                                               cpu->B) != 0)
                        return T425_STEP_BADMEM;
                }
            }

            cpu->B = cpu->C;
            cpu->C = 0u;

            finish(core, 8u);
            return T425_STEP_OK;
        }

        case 0x51u: /* TALTWT */
        {
            uint32_t state;
            uint32_t tlink;
            uint32_t time;
            uint32_t now;

            if (t425_memory_write_word(&core->memory,
                                       cpu->Wptr,
                                       UINT32_MAX) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_read_word(&core->memory,
                                      cpu->Wptr - 12u,
                                      &state) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_read_word(&core->memory,
                                      cpu->Wptr - 16u,
                                      &tlink) != 0)
                return T425_STEP_BADMEM;

            if (t425_memory_read_word(&core->memory,
                                      cpu->Wptr - 20u,
                                      &time) != 0)
                return T425_STEP_BADMEM;

            if (t425_wdesc_priority(core->scheduler.current_wdesc) ==
                T425_PRIORITY_HIGH)
                now = t425_timer_read_high(&core->timer);
            else
                now = t425_timer_read_low(&core->timer);

            /*
             * A ready guard has already selected the ALT branch.
             */
            if (state == T425_PROCESS_STATE_READY)
            {
                /*
                 * TALTWT: when an ALT guard is already ready,
                 * record the current priority timer in pw.Time.
                 */
                if (t425_memory_write_word(
                        &core->memory,
                        cpu->Wptr - 20u,
                        now) != 0)
                    return T425_STEP_BADMEM;

                finish(core, 15u);
                return T425_STEP_OK;
            }

            /*
             * A timer guard has selected the branch only when the
             * timer has expired. Equality is considered reached here.
             */
            if (tlink == T425_TIMER_TIME_SET &&
                (int32_t)(now - time) >= 0)
            {
                /*
                 * TALTWT: an already-expired timer guard selects
                 * the ALT branch immediately. The process remains
                 * running, becomes Ready, and pw.Time records the
                 * current priority timer value.
                 */
                if (t425_memory_write_word(
                        &core->memory,
                        cpu->Wptr - 12u,
                        T425_PROCESS_STATE_READY) != 0)
                    return T425_STEP_BADMEM;

                if (t425_memory_write_word(
                        &core->memory,
                        cpu->Wptr - 20u,
                        now) != 0)
                    return T425_STEP_BADMEM;

                finish(core, 15u);
                return T425_STEP_OK;
            }

            if (core->scheduler.current_wdesc == T425_NOT_PROCESS)
                return T425_STEP_BADMEM;

            /*
             * The process is waiting for the timer ALT. Transition
             * the process to Waiting, save the instruction pointer,
             * and insert it in the appropriate timer queue.
             */
            if (t425_memory_write_word(&core->memory,
                                       cpu->Wptr - 12u,
                                       T425_PROCESS_STATE_WAITING) != 0)
                return T425_STEP_BADMEM;
            if (t425_memory_write_word(&core->memory,
                                       cpu->Wptr - 4u,
                                       cpu->Iptr) != 0)
                return T425_STEP_BADMEM;

            if (tlink != T425_TIMER_TIME_SET)
                return T425_STEP_BADMEM;

            if (t425_timer_enqueue(core,
                                   core->scheduler.current_wdesc,
                                   time,
                                   1) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            if (t425_schedule_next(core) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            cpu->A = 0u;
            cpu->B = 0u;
            cpu->C = 0u;

            finish(core, 48u);
            return T425_STEP_OK;
        }

        case 0x2Eu: /* DIST */
        {
            uint32_t disable_status;
            uint32_t now;
            int guard_fired;

            if (t425_memory_read_word(&core->memory,
                                      cpu->Wptr,
                                      &disable_status) != 0)
                return T425_STEP_BADMEM;

            if (t425_wdesc_priority(core->scheduler.current_wdesc) ==
                T425_PRIORITY_HIGH)
                now = t425_timer_read_high(&core->timer);
            else
                now = t425_timer_read_low(&core->timer);

            /*
             * Timer guard fires only if:
             *   B != 0
             *   no other ALT guard has been selected
             *   current time is strictly after C.
             */
            guard_fired =
                (cpu->B != 0u) &&
                (disable_status == UINT32_MAX) &&
                ((int32_t)(now - cpu->C) > 0);

            if (guard_fired)
            {
                if (t425_memory_write_word(&core->memory,
                                           cpu->Wptr,
                                           cpu->A) != 0)
                    return T425_STEP_BADMEM;

                cpu->A = 1u;
            }
            else
            {
                /*
                 * If this timer guard is still in the future, remove
                 * this ALT process from the timer queue.
                 */
                if (cpu->B != 0u &&
                    disable_status == UINT32_MAX &&
                    (int32_t)(cpu->C - now) > 0)
                {
                    if (core->scheduler.current_wdesc ==
                        T425_NOT_PROCESS)
                        return T425_STEP_BADMEM;

                    if (t425_timer_remove(
                            core,
                            core->scheduler.current_wdesc) !=
                        T425_STEP_OK)
                        return T425_STEP_BADMEM;
                }

                cpu->A = 0u;
            }

            /*
             * B and C are undefined after DIST.
             * Keep deterministic zero values in the simulator.
             */
            cpu->B = 0u;
            cpu->C = 0u;

            finish(core, 23u);
            return T425_STEP_OK;
        }

        case 0x2Bu: /* TIN */
        {
            uint32_t now;
            if (t425_wdesc_priority(core->scheduler.current_wdesc) == T425_PRIORITY_HIGH)
                now = t425_timer_read_high(&core->timer);
            else
                now = t425_timer_read_low(&core->timer);
            if ((int32_t)(cpu->A - now) < 0)
            {
                finish(core, 4u);
                return T425_STEP_OK;
            }

            if (core->scheduler.current_wdesc == T425_NOT_PROCESS)
                return T425_STEP_BADMEM;

            if (t425_memory_write_word(&core->memory,
                                       cpu->Wptr - 4u,
                                       cpu->Iptr) != 0)
                return T425_STEP_BADMEM;

            if (t425_timer_enqueue(core,
                                   core->scheduler.current_wdesc,
                                   cpu->A,
                                   0) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            if (t425_schedule_next(core) != T425_STEP_OK)
                return T425_STEP_BADMEM;

            cpu->A = 0u;
            cpu->B = 0u;
            cpu->C = 0u;
            finish(core, 30u);
            return T425_STEP_OK;
        }
        case 0x22u: /* LDTIMER */
            if (t425_wdesc_priority(core->scheduler.current_wdesc)
                == T425_PRIORITY_HIGH)
                push(cpu, t425_timer_read_high(&core->timer));
            else
                push(cpu, t425_timer_read_low(&core->timer));
            finish(core, 2u);
            return T425_STEP_OK;

        case 0x54u: /* STTIMER */
        {
            /*
             * T425 STTIMER:
             *
             *   ClockReg[High] = A
             *   ClockReg[Low]  = A
             *   A = B
             *   B = C
             *   C = undefined
             *
             * The simulator keeps C deterministic as zero.
             * STTIMER also starts the timer clocks.
             */
            core->timer.high = cpu->A;
            core->timer.low = cpu->A;

            cpu->A = cpu->B;
            cpu->B = cpu->C;
            cpu->C = 0u;

            core->timer.running = 1u;

            finish(core, 1u);
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
