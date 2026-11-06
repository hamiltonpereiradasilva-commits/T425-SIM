#include "t425_vm_user_link.h"




static int user_link_tx(void *context, uint8_t byte)
{
    t425_vm_user_link_t *link = context;
    if (link == NULL || link->core_to_host_pending) return -1;
    link->core_to_host_byte = byte;
    link->core_to_host_pending = true;
    return 0;
}

static bool user_link_rx_available(void *context)
{
    const t425_vm_user_link_t *link = context;
    return link != NULL && link->host_to_core_pending;
}

static int user_link_rx_get(void *context, uint8_t *byte)
{
    t425_vm_user_link_t *link = context;
    if (!user_link_rx_available(context) || byte == NULL) return -1;
    *byte = link->host_to_core_byte;
    {}
    link->host_to_core_pending = false;
    return 0;
}

bool t425_vm_user_link_init(
    t425_vm_user_link_t *user_link,
    t425_vm_t *vm,
    t425_vm_transport_t *transport,
    t425_vm_endpoint_t endpoint)
{
    if (user_link == NULL || vm == NULL || transport == NULL ||
        transport->vm != vm || !t425_vm_endpoint_valid(&endpoint))
        return false;
    if (t425_vm_find_node(vm, endpoint.node) == NULL) return false;
    *user_link = (t425_vm_user_link_t){0};
    user_link->vm = vm;
    user_link->transport = transport;
    user_link->endpoint = endpoint;
    if (!t425_vm_transport_bind_external(
            transport, endpoint, user_link,
            user_link_tx, user_link_rx_available, user_link_rx_get)) {
        *user_link = (t425_vm_user_link_t){0};
        return false;
    }
    user_link->initialized = true;
    return true;
}

void t425_vm_user_link_destroy(t425_vm_user_link_t *user_link)
{
    if (user_link == NULL) return;
    if (user_link->transport != NULL)
        t425_vm_transport_unbind_external(user_link->transport);
    *user_link = (t425_vm_user_link_t){0};
}

t425_vm_user_link_result_t t425_vm_user_link_write_byte(
    t425_vm_user_link_t *user_link, uint8_t byte)
{
    if (user_link == NULL || !user_link->initialized)
        return T425_VM_USER_LINK_INVALID;
    if (user_link->host_to_core_pending)
        return T425_VM_USER_LINK_BUSY;
    user_link->host_to_core_byte = byte;
    user_link->host_to_core_pending = true;
    return T425_VM_USER_LINK_OK;
}

bool t425_vm_user_link_write_ready(
    const t425_vm_user_link_t *user_link)
{
    return user_link != NULL && user_link->initialized &&
           !user_link->host_to_core_pending;
}

bool t425_vm_user_link_read_available(
    const t425_vm_user_link_t *user_link)
{
    return user_link != NULL && user_link->initialized &&
           user_link->core_to_host_pending;
}

t425_vm_user_link_result_t t425_vm_user_link_read_byte(
    t425_vm_user_link_t *user_link, uint8_t *byte)
{
    if (user_link == NULL || !user_link->initialized || byte == NULL)
        return T425_VM_USER_LINK_INVALID;
    if (!user_link->core_to_host_pending)
        return T425_VM_USER_LINK_BUSY;
    *byte = user_link->core_to_host_byte;
    user_link->core_to_host_pending = false;
    return T425_VM_USER_LINK_OK;
}
