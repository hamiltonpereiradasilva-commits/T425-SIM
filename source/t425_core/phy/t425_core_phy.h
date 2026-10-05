#ifndef T425_CORE_PHY_H
#define T425_CORE_PHY_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define T425_CORE_PHY_MAX_ENDPOINTS 4u
#define T425_CORE_PHY_MAX_LINKS      4u

typedef int (*t425_core_phy_tx_fn)(
    void *context,
    uint32_t endpoint_id,
    uint16_t frame);

typedef bool (*t425_core_phy_rx_available_fn)(
    void *context,
    uint32_t endpoint_id);

typedef int (*t425_core_phy_rx_get_fn)(
    void *context,
    uint32_t endpoint_id,
    uint32_t *frame);

typedef struct
{
    bool reset;
    bool analyse;
    bool error_in;
    bool boot_from_rom;
    bool event_req;
    bool disable_int_ram;
    bool proc_speed_select[3];
    bool link_special;
    bool link0_special;
    bool link123_special;
} t425_core_phy_system_inputs_t;

typedef struct
{
    bool error;
    bool event_ack;
    bool event_waiting;
} t425_core_phy_system_outputs_t;

typedef bool (*t425_core_phy_read_system_inputs_fn)(
    void *context,
    t425_core_phy_system_inputs_t *inputs);

typedef bool (*t425_core_phy_write_system_outputs_fn)(
    void *context,
    const t425_core_phy_system_outputs_t *outputs);

typedef int (*t425_core_phy_set_rx_admission_fn)(
    void *context,
    uint32_t endpoint_id,
    bool enabled);

typedef int (*t425_core_phy_set_rx_admission_credit_fn)(
    void *context,
    uint32_t endpoint_id,
    uint32_t generation,
    bool enabled);

typedef int (*t425_core_phy_set_rx_arbitration_fn)(
    void *context,
    bool wait_control);

typedef struct
{
    t425_core_phy_tx_fn tx_frame;
    t425_core_phy_rx_available_fn rx_available;
    t425_core_phy_rx_get_fn rx_get;
    void *context;
    t425_core_phy_read_system_inputs_fn read_system_inputs;
    t425_core_phy_write_system_outputs_fn write_system_outputs;
    t425_core_phy_set_rx_admission_fn set_rx_admission;
    t425_core_phy_set_rx_admission_credit_fn set_rx_admission_credit;
    t425_core_phy_set_rx_arbitration_fn set_rx_arbitration;
} t425_core_phy_ops_t;

typedef struct
{
    t425_core_phy_ops_t ops;
    uint32_t endpoint_count;
    bool attached;
} t425_core_phy_t;

bool t425_core_phy_init(
    t425_core_phy_t *phy,
    const t425_core_phy_ops_t *ops,
    uint32_t endpoint_count);

int t425_core_phy_tx(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint16_t frame);

bool t425_core_phy_rx_available(
    t425_core_phy_t *phy,
    uint32_t endpoint_id);

int t425_core_phy_rx(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint32_t *frame);

bool t425_core_phy_read_system_inputs(
    t425_core_phy_t *phy,
    t425_core_phy_system_inputs_t *inputs);

bool t425_core_phy_write_system_outputs(
    t425_core_phy_t *phy,
    const t425_core_phy_system_outputs_t *outputs);

int t425_core_phy_set_rx_admission(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    bool enabled);

int t425_core_phy_set_rx_admission_credit(
    t425_core_phy_t *phy,
    uint32_t endpoint_id,
    uint32_t generation,
    bool enabled);

int t425_core_phy_set_rx_arbitration(
    t425_core_phy_t *phy,
    bool wait_control);

#ifdef __cplusplus
}
#endif

#endif
