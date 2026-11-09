#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_memory.h"

typedef struct {
    unsigned calls;
    unsigned reads;
    unsigned writes;
    uint32_t last_address;
    size_t last_size;
    uint8_t last_data[4];
    uint8_t e8[4];
} fake_backend_t;

static int fake_read(void *opaque, uint32_t address, uint8_t *data, size_t size)
{
    fake_backend_t *fake = opaque;
    fake->calls++;
    fake->reads++;
    fake->last_address = address;
    fake->last_size = size;
    if (address == 0xE8u && size == 4u)
        memcpy(data, fake->e8, 4u);
    else
        memset(data, 0, size);
    memcpy(fake->last_data, data, size < 4u ? size : 4u);
    return 0;
}

static int fake_write(void *opaque, uint32_t address, const uint8_t *data, size_t size)
{
    fake_backend_t *fake = opaque;
    fake->calls++;
    fake->writes++;
    fake->last_address = address;
    fake->last_size = size;
    memcpy(fake->last_data, data, size < 4u ? size : 4u);
    if (address == 0xE8u && size == 4u)
        memcpy(fake->e8, data, 4u);
    return 0;
}

static void clear_call(fake_backend_t *fake)
{
    fake->calls = fake->reads = fake->writes = 0u;
    fake->last_address = 0u;
    fake->last_size = 0u;
    memset(fake->last_data, 0, sizeof fake->last_data);
}

int main(void)
{
    t425_memory_t memory;
    fake_backend_t fake = {0};
    uint8_t byte = 0u;
    uint32_t word = 0u;
    uint8_t before[4];

    memset(&memory, 0xA5, sizeof memory);
    t425_memory_reset(&memory);
    assert(t425_memory_read_byte(&memory, 0xE8u, &byte) != 0);
    assert(t425_memory_write_byte(&memory, 0xE8u, 0x11u) != 0);
    assert(t425_memory_read_word(&memory, 0xE8u, &word) != 0);
    assert(t425_memory_write_word(&memory, 0xE8u, 0x80000171u) != 0);
    puts("TEST_A=PASS");

    t425_memory_set_external_backend(&memory, fake_read, fake_write, &fake);
    clear_call(&fake);
    assert(t425_memory_write_word(&memory, 0xE8u, 0x80000171u) == 0);
    assert(fake.calls == 1u && fake.writes == 1u && fake.last_address == 0xE8u && fake.last_size == 4u);
    assert(memcmp(fake.last_data, (uint8_t[]){0x71,0x01,0x00,0x80}, 4u) == 0);
    assert(t425_memory_read_word(&memory, 0xE8u, &word) == 0 && word == 0x80000171u);
    puts("TEST_B=PASS");

    clear_call(&fake);
    assert(t425_memory_write_byte(&memory, 0x80000000u, 0x12u) == 0);
    assert(t425_memory_write_byte(&memory, 0x80000070u, 0x34u) == 0);
    assert(t425_memory_write_word(&memory, 0x80000FFCu, 0x44332211u) == 0);
    assert(fake.calls == 0u);
    puts("TEST_C=PASS");

    clear_call(&fake); assert(t425_memory_read_byte(&memory, 0x80001000u, &byte) == 0 && fake.last_address == 0x80001000u); puts("TEST_D=PASS");
    clear_call(&fake); assert(t425_memory_read_byte(&memory, 0x00000000u, &byte) == 0 && fake.last_address == 0x00000000u); puts("TEST_E=PASS");
    clear_call(&fake); assert(t425_memory_read_byte(&memory, 0x7FFFFFFFu, &byte) == 0 && fake.last_address == 0x7FFFFFFFu); puts("TEST_F=PASS");
    clear_call(&fake); assert(t425_memory_read_byte(&memory, 0xFFFFFFFFu, &byte) == 0 && fake.last_address == 0xFFFFFFFFu); puts("TEST_G=PASS");

    memcpy(before, &memory.data[0xFFCu], 4u);
    clear_call(&fake);
    assert(t425_memory_read_word(&memory, 0x80000FFEu, &word) != 0);
    assert(t425_memory_write_word(&memory, 0x80000FFEu, 0xAABBCCDDu) != 0);
    assert(fake.calls == 0u && memcmp(before, &memory.data[0xFFCu], 4u) == 0);
    puts("TEST_H=PASS");

    clear_call(&fake);
    assert(t425_memory_read_word(&memory, 0x80000FFCu, &word) == 0 && word == 0x44332211u && fake.calls == 0u);
    puts("TEST_I=PASS");

    clear_call(&fake);
    for (uint32_t address = 0u; address <= 0x80001000u; address = address == 0x80001000u ? 0xFFFFFFFFu : address + 0x7FFFFFFFu) {
        assert(t425_memory_read_byte(&memory, address, &byte) == 0);
        assert(fake.last_address == address);
        if (address == 0xFFFFFFFFu) break;
    }
    assert(t425_memory_read_byte(&memory, 0xE8u, &byte) == 0);
    assert(t425_memory_read_byte(&memory, 0x7FFFFFFFu, &byte) == 0);
    assert(t425_memory_read_byte(&memory, 0x80001000u, &byte) == 0);
    assert(t425_memory_read_byte(&memory, 0xFFFFFFFFu, &byte) == 0);
    puts("TEST_J=PASS");
    puts("MEM01E=PASS");
    return 0;
}
