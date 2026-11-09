#include <string.h>
#include "t425_vm_memory_manager.h"

static int access_end_valid(uint32_t address, size_t size)
{
    if (size == 0u)
        return 0;
    return (size - 1u) <= (size_t)(UINT32_MAX - address);
}

static t425_vm_memory_region_t *find_region(
    t425_vm_memory_manager_t *manager, uint32_t address, size_t size,
    uint32_t permission)
{
    if (!manager || !access_end_valid(address, size))
        return NULL;

    for (size_t i = 0; i < manager->region_count; ++i) {
        t425_vm_memory_region_t *region = &manager->regions[i];
        uint32_t offset;
        if ((region->flags & permission) != permission ||
            address < region->t425_base)
            continue;
        offset = address - region->t425_base;
        if (offset <= region->size && size <= (size_t)(region->size - offset))
            return region;
    }
    return NULL;
}

int t425_vm_memory_manager_init(t425_vm_memory_manager_t *manager)
{
    if (!manager)
        return -1;
    memset(manager, 0, sizeof *manager);
    return 0;
}

int t425_vm_memory_add_region(
    t425_vm_memory_manager_t *manager,
    uint32_t t425_base,
    uint32_t size,
    uint8_t *backing,
    uint32_t flags)
{
    if (!manager || manager->region_count >= T425_VM_MEMORY_MAX_REGIONS ||
        size == 0u || !backing || (flags & (T425_VM_MEMORY_READ | T425_VM_MEMORY_WRITE)) == 0u ||
        size > UINT32_MAX - t425_base)
        return -1;

    for (size_t i = 0; i < manager->region_count; ++i) {
        const t425_vm_memory_region_t *other = &manager->regions[i];
        uint32_t end = t425_base + size;
        uint32_t other_end = other->t425_base + other->size;
        if (t425_base < other_end && other->t425_base < end)
            return -1;
    }

    manager->regions[manager->region_count++] =
        (t425_vm_memory_region_t){t425_base, size, backing, flags};
    return 0;
}

int t425_vm_memory_manager_read(
    t425_vm_memory_manager_t *manager,
    uint32_t address,
    uint8_t *data,
    size_t size)
{
    t425_vm_memory_region_t *region;
    if (!data || !(region = find_region(manager, address, size, T425_VM_MEMORY_READ)))
        return -1;
    memcpy(data, region->backing + (address - region->t425_base), size);
    return 0;
}

int t425_vm_memory_manager_write(
    t425_vm_memory_manager_t *manager,
    uint32_t address,
    const uint8_t *data,
    size_t size)
{
    t425_vm_memory_region_t *region;
    if (!data || !(region = find_region(manager, address, size, T425_VM_MEMORY_WRITE)))
        return -1;
    memcpy(region->backing + (address - region->t425_base), data, size);
    return 0;
}

int t425_vm_memory_manager_external_read(
    void *context, uint32_t address, uint8_t *data, size_t size)
{
    return t425_vm_memory_manager_read(
        (t425_vm_memory_manager_t *)context, address, data, size);
}

int t425_vm_memory_manager_external_write(
    void *context, uint32_t address, const uint8_t *data, size_t size)
{
    return t425_vm_memory_manager_write(
        (t425_vm_memory_manager_t *)context, address, data, size);
}
