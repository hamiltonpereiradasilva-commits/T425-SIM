#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "t425_memory.h"
#include "t425_vm_memory_manager.h"

int main(void)
{
    t425_vm_memory_manager_t manager;
    t425_memory_t core_memory;
    uint8_t backing[0x1000] = {0};
    uint8_t small[0x100] = {0};
    uint8_t readback[4] = {0};
    uint32_t word = 0;

    assert(t425_vm_memory_manager_init(&manager) == 0);
    assert(t425_vm_memory_add_region(&manager, 0x00000000u, 0x1000u, backing, T425_VM_MEMORY_READ | T425_VM_MEMORY_WRITE) == 0);
    puts("TEST_A=PASS");

    t425_memory_reset(&core_memory);
    t425_memory_set_external_backend(&core_memory, t425_vm_memory_manager_external_read, t425_vm_memory_manager_external_write, &manager);
    assert(t425_memory_write_word(&core_memory, 0xE8u, 0x80000171u) == 0);
    assert(backing[0xE8] == 0x71 && backing[0xE9] == 0x01 && backing[0xEA] == 0x00 && backing[0xEB] == 0x80);
    assert(t425_memory_read_word(&core_memory, 0xE8u, &word) == 0 && word == 0x80000171u);
    puts("TEST_B=PASS");

    assert(t425_vm_memory_manager_read(&manager, 0xE8u, readback, 0u) != 0);
    assert(t425_vm_memory_manager_write(&manager, 0xE8u, readback, 0u) != 0);
    puts("TEST_L=PASS");

    assert(t425_vm_memory_add_region(&manager, 0x10000000u, 0x100u, small, T425_VM_MEMORY_READ | T425_VM_MEMORY_WRITE) == 0);
    assert(t425_vm_memory_manager_write(&manager, 0x10000020u, (uint8_t[]){0x5A}, 1u) == 0 && small[0x20] == 0x5A);
    puts("TEST_C=PASS");
    assert(t425_vm_memory_manager_read(&manager, 0x20000000u, readback, 1u) != 0); puts("TEST_D=PASS");
    assert(t425_vm_memory_add_region(&manager, 0x10000080u, 0x100u, small, T425_VM_MEMORY_READ) != 0); puts("TEST_E=PASS");
    assert(t425_vm_memory_add_region(&manager, 0x30000000u, 0u, small, T425_VM_MEMORY_READ) != 0); puts("TEST_F=PASS");
    assert(t425_vm_memory_add_region(&manager, 0x30000000u, 0x10u, NULL, T425_VM_MEMORY_READ) != 0); puts("TEST_G=PASS");
    assert(t425_vm_memory_add_region(&manager, 0xFFFFFFF0u, 0x20u, small, T425_VM_MEMORY_READ) != 0); puts("TEST_H=PASS");
    t425_vm_memory_manager_t cross_end;
    assert(t425_vm_memory_manager_init(&cross_end) == 0);
    assert(t425_vm_memory_add_region(&cross_end, 0x00000000u, 0x100u, small, T425_VM_MEMORY_READ) == 0);
    assert(t425_vm_memory_manager_read(&cross_end, 0xFEu, readback, 4u) != 0); puts("TEST_I=PASS");

    {
        uint8_t top_backing[2] = {0};
        t425_vm_memory_manager_t top;
        assert(t425_vm_memory_manager_init(&top) == 0);
        assert(t425_vm_memory_add_region(
                   &top, 0xFFFFFFFEu, 0x1u, top_backing,
                   T425_VM_MEMORY_READ | T425_VM_MEMORY_WRITE) == 0);
        assert(t425_vm_memory_manager_write(&top, 0xFFFFFFFEu, (uint8_t[]){0xA5u}, 1u) == 0);
        assert(top_backing[0] == 0xA5u);
        puts("TEST_M=PASS");
        assert(t425_vm_memory_manager_read(&top, 0xFFFFFFFEu, readback, 2u) != 0);
        puts("TEST_N=PASS");
    }

    t425_vm_memory_manager_t readonly;
    assert(t425_vm_memory_manager_init(&readonly) == 0);
    assert(t425_vm_memory_add_region(&readonly, 0x40000000u, 0x10u, small, T425_VM_MEMORY_READ) == 0);
    assert(t425_vm_memory_manager_read(&readonly, 0x40000000u, readback, 1u) == 0);
    assert(t425_vm_memory_manager_write(&readonly, 0x40000000u, readback, 1u) != 0);
    puts("TEST_J=PASS");

    uint8_t manager_backing[0x100] = {0};
    t425_vm_memory_manager_t internal_manager;
    assert(t425_vm_memory_manager_init(&internal_manager) == 0);
    assert(t425_vm_memory_add_region(&internal_manager, 0x80000000u, 0x100u, manager_backing, T425_VM_MEMORY_READ | T425_VM_MEMORY_WRITE) == 0);
    t425_memory_reset(&core_memory);
    assert(t425_memory_write_byte(&core_memory, 0x80000070u, 0xA6u) == 0);
    assert(manager_backing[0x70] == 0u);
    puts("TEST_K=PASS");
    puts("MEM01F=PASS");
    return 0;
}
