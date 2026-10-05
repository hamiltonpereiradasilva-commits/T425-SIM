#ifndef T425_CORE_PHY_IPC_H
#define T425_CORE_PHY_IPC_H

#include <stdbool.h>
#include <stdint.h>

#include "pico/util/queue.h"
#include "t425_core_phy.h"

#define T425_CORE_PHY_IPC_QUEUE_DEPTH 8u

typedef struct
{
    uint8_t byte;
} t425_core_phy_ipc_byte_t;

/* Shared SRAM mailbox; it contains no physical-ACK message or field. */
typedef struct
{
    queue_t core0_to_core1[T425_CORE_PHY_MAX_ENDPOINTS];
    queue_t core1_to_core0[T425_CORE_PHY_MAX_ENDPOINTS];
    t425_core_phy_system_inputs_t system_inputs;
    t425_core_phy_system_outputs_t system_outputs;
    /* One atomic publication contains enabled + 31-bit generation. */
    volatile uint32_t rx_admission_word[T425_CORE_PHY_MAX_ENDPOINTS];
    volatile uint32_t rx_wait_control_arbitration;
    bool initialized;
} t425_core_phy_ipc_t;

typedef struct
{
    t425_core_phy_ipc_t *ipc;
    t425_core_phy_t phy;
    bool rx_wait_control_claimed;
    bool rx_wait_control_mode;
} t425_core_phy_ipc_service_t;

/* Called once by Core0 before launching Core1. */
bool t425_core_phy_ipc_init(t425_core_phy_ipc_t *ipc);

/* Core0 side: callbacks for t425_core_phy_init(). */
void t425_core_phy_ipc_make_core0_ops(
    t425_core_phy_ipc_t *ipc,
    t425_core_phy_ops_t *ops);

/* Core1 side: binds PHY-local adapter callbacks to the mailbox service. */
bool t425_core_phy_ipc_service_init(
    t425_core_phy_ipc_service_t *service,
    t425_core_phy_ipc_t *ipc,
    const t425_core_phy_ops_t *core1_phy_ops);

/* Call repeatedly from the Core1 link-service loop; returns work count. */
uint32_t t425_core_phy_ipc_service_poll(
    t425_core_phy_ipc_service_t *service);

#endif
