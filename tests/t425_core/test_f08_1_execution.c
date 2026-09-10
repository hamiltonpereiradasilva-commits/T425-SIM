#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "t425_core.h"

#define RAM T425_INTERNAL_RAM_BASE
#define CODE (RAM + 0x70u)
#define WORK (RAM + 0x100u)

static void fail(const char *msg)
{
    fprintf(stderr, "FAIL: %s\n", msg);
    exit(1);
}

static void expect_u32(
    const char *name,
    uint32_t actual,
    uint32_t expected)
{
    if (actual != expected)
    {
        fprintf(
            stderr,
            "FAIL: %s actual=0x%08x expected=0x%08x\n",
            name,
            actual,
            expected);
        exit(1);
    }
}

static void put8(
    t425_core_t *core,
    uint32_t address,
    uint8_t value)
{
    if (t425_memory_write_byte(
            &core->memory, address, value) != 0)
        fail("memory write");
}

static void step(t425_core_t *core)
{
    int rc = t425_core_step(core);

    if (rc != T425_STEP_OK)
    {
        fprintf(stderr, "FAIL: step returned %d\n", rc);
        exit(1);
    }
}

static void test_positive_prefix(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = CODE;

    put8(&core, CODE + 0, 0x21); /* PFIX 1 */
    put8(&core, CODE + 1, 0x22); /* PFIX 2 */
    put8(&core, CODE + 2, 0x43); /* LDC 3 */

    step(&core);
    step(&core);
    step(&core);

    expect_u32("LDC 0x123", core.cpu.A, 0x123u);
    expect_u32("Oreg clear", core.cpu.Oreg, 0u);
}

static void test_negative_prefix(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = CODE;

    put8(&core, CODE + 0, 0x61); /* NFIX 1 */
    put8(&core, CODE + 1, 0x41); /* LDC 1 */

    step(&core);
    step(&core);

    expect_u32("LDC -31", core.cpu.A, 0xffffffe1u);
    expect_u32("Oreg clear after NFIX", core.cpu.Oreg, 0u);
}

static void test_workspace_stack(void)
{
    t425_core_t core;
    uint32_t value;

    t425_core_reset(&core);
    core.cpu.Iptr = CODE;
    core.cpu.Wptr = WORK;

    put8(&core, CODE + 0, 0x4a); /* LDC 10 */
    put8(&core, CODE + 1, 0xd0); /* STL 0 */
    put8(&core, CODE + 2, 0x70); /* LDL 0 */
    put8(&core, CODE + 3, 0x8f); /* ADC 15 */

    step(&core);
    step(&core);
    step(&core);
    step(&core);

    expect_u32("workspace value", core.cpu.A, 25u);

    if (t425_memory_read_word(
            &core.memory, WORK, &value) != 0)
        fail("workspace read");

    expect_u32("STL memory", value, 10u);
}

static void test_control_flow(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = CODE;

    put8(&core, CODE + 0, 0x02); /* J 2 */

    step(&core);

    expect_u32(
        "J target",
        core.cpu.Iptr,
        CODE + 3u);
}

static void test_conditional_jump(void)
{
    t425_core_t core;

    t425_core_reset(&core);
    core.cpu.Iptr = CODE;
    core.cpu.A = 0;

    put8(&core, CODE, 0xa2); /* CJ 2 */

    step(&core);

    expect_u32(
        "CJ taken",
        core.cpu.Iptr,
        CODE + 3u);

    t425_core_reset(&core);
    core.cpu.Iptr = CODE;
    core.cpu.A = 1;
    core.cpu.B = 2;
    core.cpu.C = 3;

    put8(&core, CODE, 0xa2);

    step(&core);

    expect_u32("CJ not taken A", core.cpu.A, 2u);
    expect_u32("CJ not taken B", core.cpu.B, 3u);
}

static void test_call_ret(void)
{
    t425_core_t core;
    uint32_t saved_i;
    uint32_t saved_a;
    uint32_t saved_b;
    uint32_t saved_c;

    t425_core_reset(&core);

    core.cpu.Iptr = CODE;
    core.cpu.Wptr = WORK;
    core.cpu.A = 0x11111111u;
    core.cpu.B = 0x22222222u;
    core.cpu.C = 0x33333333u;

    put8(&core, CODE + 0, 0x94); /* CALL 4 */
    put8(&core, CODE + 5, 0x22); /* PFIX #2 */
    put8(&core, CODE + 6, 0xf0); /* OPR #0 = RET 0x20 */

    step(&core);

    expect_u32("CALL Wptr", core.cpu.Wptr, WORK - 16u);
    expect_u32("CALL target", core.cpu.Iptr, CODE + 5u);
    expect_u32("CALL A", core.cpu.A, CODE + 1u);

    if (t425_memory_read_word(
            &core.memory, WORK - 16u, &saved_i) != 0)
        fail("saved I");

    if (t425_memory_read_word(
            &core.memory, WORK - 12u, &saved_a) != 0)
        fail("saved A");

    if (t425_memory_read_word(
            &core.memory, WORK - 8u, &saved_b) != 0)
        fail("saved B");

    if (t425_memory_read_word(
            &core.memory, WORK - 4u, &saved_c) != 0)
        fail("saved C");

    expect_u32("saved I", saved_i, CODE + 1u);
    expect_u32("saved A", saved_a, 0x11111111u);
    expect_u32("saved B", saved_b, 0x22222222u);
    expect_u32("saved C", saved_c, 0x33333333u);

    step(&core); /* PFIX #2 */
    step(&core); /* OPR #0 = RET 0x20 */

    expect_u32("RET Iptr", core.cpu.Iptr, CODE + 1u);
    expect_u32("RET Wptr", core.cpu.Wptr, WORK);
    expect_u32("RET A preserved", core.cpu.A, CODE + 1u);
}

static void test_memory_endianness(void)
{
    t425_core_t core;
    uint32_t value;

    t425_core_reset(&core);

    if (t425_memory_write_word(
            &core.memory,
            RAM + 0x100u,
            0x12345678u) != 0)
        fail("word write");

    if (t425_memory_read_byte(
            &core.memory,
            RAM + 0x100u,
            (uint8_t *)&value) != 0)
        fail("byte read");

    if (t425_memory_read_word(
            &core.memory,
            RAM + 0x100u,
            &value) != 0)
        fail("word read");

    expect_u32("little endian word", value, 0x12345678u);
}

int main(void)
{
    test_positive_prefix();
    test_negative_prefix();
    test_workspace_stack();
    test_control_flow();
    test_conditional_jump();
    test_call_ret();
    test_memory_endianness();

    printf("F08.1 execution tests: PASS\n");
    return 0;
}
