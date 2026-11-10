#include <assert.h>
#include "f1_fixture_compat.h"
#include <stdint.h>
#include <stdio.h>

#include "t425_core.h"
#include "t425_memory.h"

#define CODE_BASE T425_INTERNAL_RAM_BASE

static void reset_code(t425_core_t *core)
{
    t425_core_reset(core);
    core->cpu.Iptr = CODE_BASE;
}

static void emit_op(t425_core_t *core, uint16_t code)
{
    uint8_t pfix;
    uint8_t opr;

    assert(code <= 0xFFu);

    pfix = (uint8_t)(0x20u | ((code >> 4u) & 0x0Fu));
    opr = (uint8_t)(0xF0u | (code & 0x0Fu));

    assert(t425_memory_write_byte(&core->memory,
                                  CODE_BASE,
                                  pfix) == 0);

    assert(t425_memory_write_byte(&core->memory,
                                  CODE_BASE + 1u,
                                  opr) == 0);

    assert(t425_core_step(core) == T425_STEP_OK);
    assert(t425_core_step(core) == T425_STEP_OK);
}

static uint32_t crc_reference(uint32_t data,
                              uint32_t crc,
                              uint32_t generator,
                              unsigned int bits)
{
    unsigned int i;

    for (i = 0u; i < bits; ++i)
    {
        uint32_t carry = (crc >> 31u) & 1u;

        crc = (crc << 1u) | (data >> 31u);
        data <<= 1u;

        if (carry != 0u)
            crc ^= generator;
    }

    return crc;
}

static uint32_t reverse32(uint32_t value)
{
    uint32_t result = 0u;
    unsigned int i;

    for (i = 0u; i < 32u; ++i)
    {
        result = (result << 1u) | (value & 1u);
        value >>= 1u;
    }

    return result;
}

static uint32_t reverse_n(uint32_t value, unsigned int n)
{
    uint32_t result = 0u;
    unsigned int i;

    for (i = 0u; i < n; ++i)
    {
        result = (result << 1u) | (value & 1u);
        value >>= 1u;
    }

    return result;
}

static void test_crcword(void)
{
    t425_core_t core;
    uint32_t data = 0x12345678u;
    uint32_t crc = 0x89ABCDEFu;
    uint32_t generator = 0x10210000u;
    uint32_t expected;

    reset_code(&core);

    expected = crc_reference(data, crc, generator, 32u);

    core.cpu.A = data; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = crc; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = generator; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x74u);

    assert(core.cpu.A == expected);
    assert(core.cpu.B == generator);
    assert(core.cpu.C == generator);
}

static void test_crcbyte(void)
{
    t425_core_t core;
    uint32_t data = 0xA5000000u;
    uint32_t crc = 0x13579BDFu;
    uint32_t generator = 0x10210000u;
    uint32_t expected;

    reset_code(&core);

    expected = crc_reference(data, crc, generator, 8u);

    core.cpu.A = data; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = crc; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = generator; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x75u);

    assert(core.cpu.A == expected);
    assert(core.cpu.B == generator);
    assert(core.cpu.C == generator);
}

static void test_bitcnt(void)
{
    t425_core_t core;

    reset_code(&core);

    core.cpu.A = 0xF0F00001u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 10u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x12345678u; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x76u);

    assert(core.cpu.A == 19u);
    assert(core.cpu.B == 0x12345678u);
    assert(core.cpu.C == 0x12345678u);
}

static void test_bitrevword(void)
{
    t425_core_t core;

    reset_code(&core);

    core.cpu.A = 0x0000000Du; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x11111111u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0x22222222u; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x77u);

    assert(core.cpu.A == reverse32(0x0000000Du));
    assert(core.cpu.B == 0x11111111u);
    assert(core.cpu.C == 0x22222222u);
}

static void test_bitrevnbits(void)
{
    t425_core_t core;

    reset_code(&core);

    core.cpu.A = 4u; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.B = 0x0000000Du; t425_fixture_sync_slaves(&core.cpu);
    core.cpu.C = 0xABCDEF01u; t425_fixture_sync_slaves(&core.cpu);

    emit_op(&core, 0x78u);

    assert(core.cpu.A == reverse_n(0x0000000Du, 4u));
    assert(core.cpu.A == 0x0000000Bu);
    assert(core.cpu.B == 0xABCDEF01u);
    assert(core.cpu.C == 0xABCDEF01u);
}

int main(void)
{
    test_crcword();
    test_crcbyte();
    test_bitcnt();
    test_bitrevword();
    test_bitrevnbits();

    puts("F08.1-C.6.2-B CRC/bit operations: PASS");
    return 0;
}
