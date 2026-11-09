#ifndef LINKUSB_ADAPTER_H
#define LINKUSB_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

#include "t425_link.h"
#include "linkusb_subsystem.h"

#define LINKUSB_PHYSICAL_LINK_COUNT 2u
#define LINKUSB_RX_BUFFER_WORDS     16u
#define LINKUSB_RX_SERVICE_BUDGET   2u

typedef struct {
    uint8_t data;
    bool data_pending;
    bool physical_ack_pending;
    uint32_t malformed_words;
} linkusb_link_service_state_t;

typedef struct {
    t425_link_programs_t programs;
    linkusb_subsystem_t subsystem;
    t425_link_t links[LINKUSB_PHYSICAL_LINK_COUNT];

    uint32_t rx_buffers[LINKUSB_PHYSICAL_LINK_COUNT]
                       [LINKUSB_RX_BUFFER_WORDS];

    uint32_t initialized_links;
    bool programs_loaded;
    bool started;
    linkusb_link_service_state_t service[LINKUSB_PHYSICAL_LINK_COUNT];
    uint32_t service_cursor;
} linkusb_adapter_t;

bool linkusb_adapter_init(linkusb_adapter_t *adapter);
bool linkusb_adapter_start(linkusb_adapter_t *adapter);
void linkusb_adapter_stop(linkusb_adapter_t *adapter);
void linkusb_adapter_deinit(linkusb_adapter_t *adapter);

t425_link_t *linkusb_adapter_link(
    linkusb_adapter_t *adapter,
    uint32_t link_id);

void linkusb_adapter_service(linkusb_adapter_t *adapter);
bool linkusb_adapter_rx_available(
    const linkusb_adapter_t *adapter,
    uint32_t link_id);
bool linkusb_adapter_rx_get(
    linkusb_adapter_t *adapter,
    uint32_t link_id,
    uint8_t *data);
bool linkusb_adapter_tx_ready(
    const linkusb_adapter_t *adapter,
    uint32_t link_id);
bool linkusb_adapter_tx_put(
    linkusb_adapter_t *adapter,
    uint32_t link_id,
    uint8_t data);

#endif
