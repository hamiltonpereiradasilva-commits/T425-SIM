#ifndef T425_VM_MEMORY_MANAGER_H
#define T425_VM_MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define T425_VM_MEMORY_MAX_REGIONS 8u
#define T425_VM_MEMORY_READ  (1u << 0)
#define T425_VM_MEMORY_WRITE (1u << 1)

typedef struct {
    uint32_t t425_base;
    uint32_t size;
    uint8_t *backing;
    uint32_t flags;
} t425_vm_memory_region_t;

typedef struct {
    t425_vm_memory_region_t regions[T425_VM_MEMORY_MAX_REGIONS];
    size_t region_count;
} t425_vm_memory_manager_t;

int t425_vm_memory_manager_init(t425_vm_memory_manager_t *manager);
int t425_vm_memory_add_region(
    t425_vm_memory_manager_t *manager,
    uint32_t t425_base,
    uint32_t size,
    uint8_t *backing,
    uint32_t flags);

int t425_vm_memory_manager_read(
    t425_vm_memory_manager_t *manager,
    uint32_t address,
    uint8_t *data,
    size_t size);
int t425_vm_memory_manager_write(
    t425_vm_memory_manager_t *manager,
    uint32_t address,
    const uint8_t *data,
    size_t size);

int t425_vm_memory_manager_external_read(
    void *context, uint32_t address, uint8_t *data, size_t size);
int t425_vm_memory_manager_external_write(
    void *context, uint32_t address, const uint8_t *data, size_t size);

#endif
