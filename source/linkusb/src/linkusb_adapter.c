#include "linkusb_adapter.h"

#include <string.h>

#include "hardware/pio.h"

static const uint linkusb_tx_sm[LINKUSB_PHYSICAL_LINK_COUNT] = {
    0u, 2u
};

static const uint linkusb_rx_sm[LINKUSB_PHYSICAL_LINK_COUNT] = {
    1u, 3u
};

static const uint linkusb_tx_pin[LINKUSB_PHYSICAL_LINK_COUNT] = {
    0u, 19u
};

static const uint linkusb_rx_pin[LINKUSB_PHYSICAL_LINK_COUNT] = {
    1u, 18u
};

static void linkusb_adapter_rollback(linkusb_adapter_t *adapter)
{
    if (adapter == 0)
        return;

    linkusb_subsystem_deinit(&adapter->subsystem);

    if (adapter->started) {
        for (uint32_t i = 0u;
             i < adapter->initialized_links;
             ++i) {
            t425_link_stop(&adapter->links[i]);
        }
        adapter->started = false;
    }

    for (uint32_t i = 0u;
         i < adapter->initialized_links;
         ++i) {
        t425_link_dma_deinit(&adapter->links[i]);
    }

    adapter->initialized_links = 0u;

    if (adapter->programs_loaded) {
        t425_link_programs_unload(&adapter->programs);
        adapter->programs_loaded = false;
    }
}

bool linkusb_adapter_init(linkusb_adapter_t *adapter)
{
    if (adapter == 0)
        return false;

    memset(adapter, 0, sizeof(*adapter));

    if (!linkusb_subsystem_init(&adapter->subsystem))
        return false;

    if (!t425_link_programs_load(&adapter->programs, pio0)) {
        linkusb_subsystem_deinit(&adapter->subsystem);
        return false;
    }

    adapter->programs_loaded = true;

    for (uint32_t i = 0u;
         i < LINKUSB_PHYSICAL_LINK_COUNT;
         ++i) {

        if (!t425_link_init_shared(
                &adapter->links[i],
                &adapter->programs,
                linkusb_tx_sm[i],
                linkusb_rx_sm[i],
                linkusb_tx_pin[i],
                linkusb_rx_pin[i],
                T425_LINK_SPEED_10M)) {
            linkusb_adapter_rollback(adapter);
            return false;
        }

        if (!t425_link_dma_init(
                &adapter->links[i],
                adapter->rx_buffers[i],
                LINKUSB_RX_BUFFER_WORDS)) {
            linkusb_adapter_rollback(adapter);
            return false;
        }

        ++adapter->initialized_links;
    }

    return true;
}

bool linkusb_adapter_start(linkusb_adapter_t *adapter)
{
    if (adapter == 0 ||
        !adapter->programs_loaded ||
        adapter->initialized_links != LINKUSB_PHYSICAL_LINK_COUNT ||
        adapter->started)
        return false;

    for (uint32_t i = 0u;
         i < LINKUSB_PHYSICAL_LINK_COUNT;
         ++i) {
        t425_link_start(&adapter->links[i]);
    }

    adapter->started = true;
    return true;
}

void linkusb_adapter_stop(linkusb_adapter_t *adapter)
{
    if (adapter == 0 || !adapter->started)
        return;

    for (uint32_t i = 0u;
         i < LINKUSB_PHYSICAL_LINK_COUNT;
         ++i) {
        t425_link_stop(&adapter->links[i]);
    }

    adapter->started = false;
}

void linkusb_adapter_deinit(linkusb_adapter_t *adapter)
{
    if (adapter == 0)
        return;

    linkusb_adapter_stop(adapter);

    for (uint32_t i = 0u;
         i < adapter->initialized_links;
         ++i) {
        t425_link_dma_deinit(&adapter->links[i]);
    }

    adapter->initialized_links = 0u;

    if (adapter->programs_loaded) {
        t425_link_programs_unload(&adapter->programs);
        adapter->programs_loaded = false;
    }

    linkusb_subsystem_deinit(&adapter->subsystem);
}

t425_link_t *linkusb_adapter_link(
    linkusb_adapter_t *adapter,
    uint32_t link_id)
{
    if (adapter == 0 ||
        link_id >= LINKUSB_PHYSICAL_LINK_COUNT ||
        link_id >= adapter->initialized_links)
        return 0;

    return &adapter->links[link_id];
}


#define LINKUSB_RX_START_BIT 31u
#define LINKUSB_RX_TYPE_BIT  30u
#define LINKUSB_RX_DATA_SHIFT 22u
#define LINKUSB_RX_STOP_BIT  21u
#define LINKUSB_RX_ACK_WORD  0x80000000u

static void linkusb_service_one(
    linkusb_adapter_t *adapter,
    uint32_t link_id)
{
    t425_link_t *link = &adapter->links[link_id];
    linkusb_link_service_state_t *state = &adapter->service[link_id];

    if (state->physical_ack_pending &&
        t425_link_tx_ready(link) &&
        t425_link_send_ack(link)) {
        state->physical_ack_pending = false;
    }

    /*
     * A full DATA slot means the next raw FIFO word cannot be classified
     * safely without consuming it. Leave the FIFO authoritative and wait
     * until the caller takes the pending byte.
     */
    if (state->data_pending)
        return;

    for (uint32_t budget = 0u;
         budget < LINKUSB_RX_SERVICE_BUDGET;
         ++budget) {
        uint32_t raw;

        if (!t425_link_rx_available(link))
            break;

        raw = t425_link_rx_get(link);

        if (raw == LINKUSB_RX_ACK_WORD) {
            t425_link_note_ack_received(link);
            continue;
        }

        if (((raw >> LINKUSB_RX_START_BIT) & 1u) != 0u &&
            ((raw >> LINKUSB_RX_TYPE_BIT) & 1u) != 0u &&
            ((raw >> LINKUSB_RX_STOP_BIT) & 1u) == 0u) {
            state->data =
                (uint8_t)((raw >> LINKUSB_RX_DATA_SHIFT) & 0xffu);
            state->data_pending = true;
            if (!t425_link_send_ack(link))
                state->physical_ack_pending = true;
            break;
        }

        if (state->malformed_words != UINT32_MAX)
            ++state->malformed_words;
    }
}

void linkusb_adapter_service(linkusb_adapter_t *adapter)
{
    uint32_t start;

    if (adapter == 0 ||
        !adapter->started ||
        adapter->initialized_links != LINKUSB_PHYSICAL_LINK_COUNT)
        return;

    start = adapter->service_cursor % LINKUSB_PHYSICAL_LINK_COUNT;
    adapter->service_cursor =
        (start + 1u) % LINKUSB_PHYSICAL_LINK_COUNT;

    for (uint32_t offset = 0u;
         offset < LINKUSB_PHYSICAL_LINK_COUNT;
         ++offset) {
        linkusb_service_one(
            adapter,
            (start + offset) % LINKUSB_PHYSICAL_LINK_COUNT);
    }
}

bool linkusb_adapter_rx_available(
    const linkusb_adapter_t *adapter,
    uint32_t link_id)
{
    if (adapter == 0 ||
        link_id >= LINKUSB_PHYSICAL_LINK_COUNT ||
        link_id >= adapter->initialized_links)
        return false;
    return adapter->service[link_id].data_pending;
}

bool linkusb_adapter_rx_get(
    linkusb_adapter_t *adapter,
    uint32_t link_id,
    uint8_t *data)
{
    linkusb_link_service_state_t *state;

    if (adapter == 0 ||
        data == 0 ||
        link_id >= LINKUSB_PHYSICAL_LINK_COUNT ||
        link_id >= adapter->initialized_links)
        return false;

    state = &adapter->service[link_id];
    if (!state->data_pending)
        return false;

    *data = state->data;
    state->data_pending = false;
    return true;
}

bool linkusb_adapter_tx_ready(
    const linkusb_adapter_t *adapter,
    uint32_t link_id)
{
    const t425_link_t *link;

    if (adapter == 0 ||
        !adapter->started ||
        link_id >= LINKUSB_PHYSICAL_LINK_COUNT ||
        link_id >= adapter->initialized_links)
        return false;

    link = &adapter->links[link_id];
    return t425_link_tx_ready(link) && !link->tx_waiting_ack;
}

bool linkusb_adapter_tx_put(
    linkusb_adapter_t *adapter,
    uint32_t link_id,
    uint8_t data)
{
    if (!linkusb_adapter_tx_ready(adapter, link_id))
        return false;
    return t425_link_send_frame(&adapter->links[link_id], data);
}
