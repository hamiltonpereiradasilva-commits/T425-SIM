#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"

#define RAM T425_INTERNAL_RAM_BASE

static void execute_opr(t425_core_t *core, uint8_t opr)
{
    core->memory.data[0] = (uint8_t)(0x20u | (opr >> 4));
    core->memory.data[1] = (uint8_t)(0xF0u | (opr & 0x0Fu));

    core->cpu.Iptr = RAM;

    {
        int rc1 = t425_core_step(core);
        printf("step1: rc=%d Oreg=0x%08x Iptr=0x%08x cycles=%llu\n",
               rc1,
               core->cpu.Oreg,
               core->cpu.Iptr,
               (unsigned long long)core->cycles);
        assert(rc1 == T425_STEP_OK);
    }

    {
        int rc2 = t425_core_step(core);
        printf("step2: rc=%d Oreg=0x%08x Iptr=0x%08x cycles=%llu\n",
               rc2,
               core->cpu.Oreg,
               core->cpu.Iptr,
               (unsigned long long)core->cycles);
        assert(rc2 == T425_STEP_OK);
    }
}

static void test_lmul_basic(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * LMUL:
     *   (A * B) + C_lsb
     *   A = LSW
     *   B = MSW
     */
    core.cpu.A = 0x00000003u;
    core.cpu.B = 0x00000002u;
    core.cpu.C = 0x00000000u;

    execute_opr(&core, 0x31u);

    assert(core.cpu.A == 0x00000006u);
    assert(core.cpu.B == 0x00000000u);
    assert(core.cpu.C == 0x00000000u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 34u); /* PFIX + LMUL */
    assert(core.instructions == 2u);
}

static void test_lmul_carry_in(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * LMUL:
     *   prod = (A * B) + C
     *   A = low word of result
     *   B = high word of result
     *
     * C is a full unsigned word, not merely C&1.
     */
    core.cpu.A = 0xFFFFFFFFu;
    core.cpu.B = 0x00000002u;
    core.cpu.C = 0x00000005u;

    execute_opr(&core, 0x31u);

    /*
     * 0xFFFFFFFF * 2 + 5
     * = 0x0000000200000003
     */
    assert(core.cpu.A == 0x00000003u);
    assert(core.cpu.B == 0x00000002u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 34u);
    assert(core.instructions == 2u);
}

static void test_ldiv_basic(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * LDIV:
     *   dividend = C:B
     *   divisor  = A
     *   A = quotient
     *   B = remainder
     */
    core.cpu.A = 3u;
    core.cpu.B = 20u;
    core.cpu.C = 0u;

    execute_opr(&core, 0x1Au);

    assert(core.cpu.A == 6u);
    assert(core.cpu.B == 2u);
    assert(core.cycles == 36u); /* PFIX + LDIV */
    assert(core.instructions == 2u);
}

static void test_ldiv_zero_high_word(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * A zero high word makes this an ordinary unsigned
     * single-word dividend.
     */
    core.cpu.A = 7u;
    core.cpu.B = 100u;
    core.cpu.C = 0u;

    execute_opr(&core, 0x1Au);

    assert(core.cpu.A == 14u);
    assert(core.cpu.B == 2u);
    assert(core.cycles == 36u);
    assert(core.instructions == 2u);
}

static void test_ldiv_nonzero_high_word(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * dividend = 0x00000001_00000000
     * divisor  = 3
     *
     * quotient  = 0x55555555
     * remainder = 1
     */
    core.cpu.A = 3u;
    core.cpu.B = 0u;
    core.cpu.C = 1u;

    execute_opr(&core, 0x1Au);

    assert(core.cpu.A == 0x55555555u);
    assert(core.cpu.B == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 36u);
    assert(core.instructions == 2u);
}

static void test_ldiv_maximum_valid_quotient(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * dividend = 0xFFFFFFFE_FFFFFFFF
     * divisor  = 0xFFFFFFFF
     *
     * quotient  = 0xFFFFFFFF
     * remainder = 0xFFFFFFFE
     *
     * This is the largest possible 32-bit quotient
     * without LDIV overflow.
     */
    core.cpu.A = 0xFFFFFFFFu;
    core.cpu.B = 0xFFFFFFFFu;
    core.cpu.C = 0xFFFFFFFEu;

    execute_opr(&core, 0x1Au);

    assert(core.cpu.A == 0xFFFFFFFFu);
    assert(core.cpu.B == 0xFFFFFFFEu);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 36u);
    assert(core.instructions == 2u);
}

static void test_ldiv_overflow(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * C >= A means the unsigned quotient cannot fit
     * in one 32-bit word.
     */
    core.cpu.A = 3u;
    core.cpu.B = 0u;
    core.cpu.C = 3u;

    execute_opr(&core, 0x1Au);

    assert(core.cpu.error == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 36u);
    assert(core.instructions == 2u);
}

static void test_ldiv_zero_divisor(void)
{
    t425_core_t core;
    t425_core_reset(&core);

    /*
     * Division by zero must signal Error.
     */
    core.cpu.A = 0u;
    core.cpu.B = 0x12345678u;
    core.cpu.C = 0u;

    execute_opr(&core, 0x1Au);

    assert(core.cpu.error == 1u);
    assert(core.cpu.Oreg == 0u);
    assert(core.cycles == 36u);
    assert(core.instructions == 2u);
}

int main(void)
{
    test_lmul_basic();
    test_lmul_carry_in();
    test_ldiv_basic();
    test_ldiv_zero_high_word();
    test_ldiv_nonzero_high_word();
    test_ldiv_maximum_valid_quotient();
    test_ldiv_overflow();
    test_ldiv_zero_divisor();

    puts("F08.1-B.2.6 LMUL/LDIV tests: PASS");
    return 0;
}
