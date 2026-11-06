#include "t425_channel_dma_memory_04.h"

#include <string.h>


typedef struct {
    t425_channel_dma_memory_04_t *adapter;
    const t425_channel_compat_input_key_t *session;
    uint32_t request_id;
    uint32_t address;
    uint32_t data;
    uint32_t mask;
} compat_rx_transaction_t;

static t425_channel_dma_lane_t *lane_for(
    t425_channel_dma_bank_t *bank,
    t425_channel_dma_direction_t direction,
    uint8_t slot)
{
    if (bank == NULL || slot >= T425_CHANNEL_DMA_SLOT_COUNT)
        return NULL;
    if (direction == T425_CHANNEL_DMA_RX)
        return &bank->rx[slot];
    if (direction == T425_CHANNEL_DMA_TX)
        return &bank->tx[slot];
    return NULL;
}

/* The sole F04 masked RX memory effect shared by legacy and compat callers. */
static bool execute_rx_masked_write(
    t425_channel_dma_memory_04_t *adapter,
    const t425_channel_dma_ubus_request_t *request,
    t425_channel_dma_ubus_response_t *response,
    bool advance_legacy_ptr)
{
    t425_channel_dma_lane_t *lane;
    int rc;

    if (adapter == NULL || adapter->memory == NULL || adapter->bank == NULL ||
        request == NULL || response == NULL ||
        request->direction != T425_CHANNEL_DMA_RX ||
        request->operation != T425_CHANNEL_DMA_UBUS_WRITE_WORD ||
        request->request_id == 0u ||
        (request->aligned_address & 3u) != 0u ||
        request->byte_mask == 0u ||
        request->byte_mask > T425_CHANNEL_DMA_UBUS_FULL_MASK) {
        if (response != NULL)
            response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_ERROR;
        return true;
    }

    lane = lane_for(adapter->bank, T425_CHANNEL_DMA_RX, request->slot);
    if (lane == NULL || (advance_legacy_ptr &&
        (lane->ptr_reg != request->aligned_address ||
         lane->ptr_reg > UINT32_MAX - 4u))) {
        response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_ERROR;
        return true;
    }

    /* A denied admission remains a RETRY with no memory effect. */
    if (adapter->admit != NULL &&
        !adapter->admit(adapter->admit_context, request)) {
        return false;
    }

    rc = t425_memory_write_masked_word(
        adapter->memory, request->aligned_address,
        request->write_data, request->byte_mask);
    if (rc != 0) {
        response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_ERROR;
        return true;
    }

    if (advance_legacy_ptr)
        lane->ptr_reg += 4u;
    response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_COMPLETE;
    return true;
}

static bool transact_memory(
    void *context,
    const t425_channel_dma_ubus_request_t *request,
    t425_channel_dma_ubus_response_t *response)
{
    t425_channel_dma_memory_04_t *adapter =
        (t425_channel_dma_memory_04_t *)context;
    t425_channel_dma_lane_t *lane;
    uint32_t read_data = 0u;
    int rc;

    if (response == NULL)
        return true;

    response->request_id = request != NULL ? request->request_id : 0u;
    response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_ERROR;
    response->read_data = 0u;

    if (adapter == NULL || adapter->memory == NULL || adapter->bank == NULL ||
        request == NULL || (request->aligned_address & 3u) != 0u)
        return true;

    if (request->direction == T425_CHANNEL_DMA_RX &&
        request->operation == T425_CHANNEL_DMA_UBUS_WRITE_WORD &&
        request->byte_mask != 0u &&
        request->byte_mask <= T425_CHANNEL_DMA_UBUS_FULL_MASK) {
        return execute_rx_masked_write(adapter, request, response, true);
    }

    lane = lane_for(adapter->bank, request->direction, request->slot);
    if (lane == NULL || (uint64_t)lane->ptr_reg != request->aligned_address ||
        lane->ptr_reg > UINT32_MAX - 4u)
        return true;

    if (adapter->admit != NULL &&
        !adapter->admit(adapter->admit_context, request))
        return false;

    if (request->direction == T425_CHANNEL_DMA_TX &&
        request->operation == T425_CHANNEL_DMA_UBUS_READ_WORD &&
        request->byte_mask == T425_CHANNEL_DMA_UBUS_FULL_MASK) {
        rc = t425_memory_read_word(
            adapter->memory, request->aligned_address, &read_data);
        if (rc != 0)
            return true;

        response->read_data = read_data;
        {}
        response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_COMPLETE;
        return true;
    }

    return true;
}

static bool compat_route_valid(
    const t425_channel_compat_input_route_t *route)
{
    uint32_t offset;
    uint8_t lane;
    if (route == NULL || route->architectural_channel < UINT32_C(0x80000010) ||
        route->architectural_channel > UINT32_C(0x8000001C) ||
        (route->architectural_channel & 3u) != 0u) {
        return false;
    }
    offset = route->architectural_channel - UINT32_C(0x80000010);
    lane = (uint8_t)(offset / 4u);
    return lane < T425_CHANNEL_COMPAT_INPUT_LANES &&
        route->input_lane == lane && route->stack_lane == lane &&
        route->shared_index == (uint8_t)(lane + T425_CHANNEL_COMPAT_INPUT_LANES);
}

static bool compat_key_equal(
    const t425_channel_compat_input_key_t *a,
    const t425_channel_compat_input_key_t *b)
{
    return a->request_id == b->request_id && a->generation == b->generation &&
        a->route.architectural_channel == b->route.architectural_channel &&
        a->route.input_lane == b->route.input_lane &&
        a->route.stack_lane == b->route.stack_lane &&
        a->route.shared_index == b->route.shared_index &&
        a->route.endpoint_id == b->route.endpoint_id &&
        a->route.physical_link == b->route.physical_link &&
        a->route.lane_id == b->route.lane_id;
}

static bool transact_compat_rx(
    void *context,
    const t425_channel_dma_ubus_request_t *request,
    t425_channel_dma_ubus_response_t *response)
{
    compat_rx_transaction_t *transaction =
        (compat_rx_transaction_t *)context;

    if (response == NULL)
        return true;
    response->request_id = request != NULL ? request->request_id : 0u;
    response->status = T425_CHANNEL_DMA_UBUS_RESPONSE_ERROR;
    response->read_data = 0u;

    if (transaction == NULL || transaction->adapter == NULL ||
        transaction->session == NULL || request == NULL ||
        request->request_id != transaction->request_id ||
        request->direction != T425_CHANNEL_DMA_RX ||
        request->operation != T425_CHANNEL_DMA_UBUS_WRITE_WORD ||
        request->slot != transaction->session->route.stack_lane ||
        request->aligned_address != transaction->address ||
        request->write_data != transaction->data ||
        request->byte_mask != transaction->mask) {
        return true;
    }

    return execute_rx_masked_write(
        transaction->adapter, request, response, false);
}

bool t425_channel_dma_memory_04_init(
    t425_channel_dma_memory_04_t *adapter,
    t425_memory_t *memory,
    t425_channel_dma_bank_t *bank)
{
    if (adapter == NULL || memory == NULL || bank == NULL)
        return false;

    memset(adapter, 0, sizeof(*adapter));
    adapter->memory = memory;
    adapter->bank = bank;
    return true;
}

void t425_channel_dma_memory_04_set_admission(
    t425_channel_dma_memory_04_t *adapter,
    t425_channel_dma_memory_04_admit_fn admit,
    void *context)
{
    if (adapter == NULL)
        return;

    adapter->admit = admit;
    adapter->admit_context = context;
}

t425_channel_dma_ubus_transfer_result_t
 t425_channel_dma_memory_04_write_rx_bytes(
    t425_channel_dma_memory_04_t *adapter,
    uint8_t slot,
    uint32_t address,
    const uint8_t *bytes,
    uint32_t byte_count,
    uint32_t first_request_id)
{
    t425_channel_dma_ubus_transfer_result_t invalid = {
        T425_CHANNEL_DMA_UBUS_TRANSFER_INVALID, 0u, 0u
    };
    t425_channel_dma_lane_t *lane;

    if (adapter == NULL || adapter->memory == NULL || adapter->bank == NULL)
        return invalid;

    lane = lane_for(adapter->bank, T425_CHANNEL_DMA_RX, slot);
    if (lane == NULL)
        return invalid;

    if (byte_count == 0u) {
        invalid.status = T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE;
        return invalid;
    }

    if (bytes == NULL ||
        (uint64_t)lane->ptr_reg != ((uint64_t)address & ~UINT64_C(3)))
        return invalid;

    return t425_channel_dma_ubus_write_rx_bytes(
        adapter->bank, slot, address, bytes, byte_count, first_request_id,
        transact_memory, adapter);
}

/* Compat commits use the same UBUS request/response and F04 masked writer,
 * but deliberately do not mutate the legacy F17 PtrReg. CHANNEL consumes
 * the returned receipt and owns progress for its isolated INPUT context. */
t425_channel_dma_ubus_transfer_status_t
 t425_channel_dma_memory_04_rx_commit_compat(
    t425_channel_dma_memory_04_t *adapter,
    const t425_channel_compat_input_key_t *session,
    uint32_t ubus_request_id,
    uint32_t aligned_address,
    uint32_t write_data,
    uint8_t byte_mask,
    t425_channel_compat_input_commit_receipt_t *receipt)
{
    compat_rx_transaction_t transaction;
    t425_channel_dma_ubus_transfer_status_t status;

    if (receipt == NULL)
        return T425_CHANNEL_DMA_UBUS_TRANSFER_INVALID;
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = T425_CHANNEL_COMPAT_COMMIT_ERROR;
    if (session == NULL ||
        !compat_route_valid(&session->route) ||
        session->request_id == 0u || session->generation == 0u ||
        ubus_request_id == 0u || (aligned_address & 3u) != 0u ||
        byte_mask == 0u || byte_mask > T425_CHANNEL_DMA_UBUS_FULL_MASK ||
        adapter == NULL || adapter->memory == NULL || adapter->bank == NULL) {
        return T425_CHANNEL_DMA_UBUS_TRANSFER_INVALID;
    }

    receipt->session = *session;
    receipt->ubus_request_id = ubus_request_id;
    receipt->aligned_address = aligned_address;
    receipt->write_data = write_data;
    receipt->byte_mask = byte_mask;

    {
        uint8_t lane = session->route.input_lane;
        if (adapter->compat_last_valid[lane] != 0u &&
            adapter->compat_last_request_id[lane] == ubus_request_id) {
            if (!compat_key_equal(&adapter->compat_last_session[lane], session) ||
                adapter->compat_last_address[lane] != aligned_address ||
                adapter->compat_last_data[lane] != write_data ||
                adapter->compat_last_mask[lane] != byte_mask) {
                return T425_CHANNEL_DMA_UBUS_TRANSFER_ERROR;
            }
            receipt->status = T425_CHANNEL_COMPAT_COMMIT_SUCCESS;
            return T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE;
        }
    }

    transaction.adapter = adapter;
    transaction.session = session;
    transaction.request_id = ubus_request_id;
    transaction.address = aligned_address;
    transaction.data = write_data;
    transaction.mask = byte_mask;
    status = t425_channel_dma_ubus_write_masked_word(
        adapter->bank, session->route.stack_lane, aligned_address,
        write_data, byte_mask, ubus_request_id,
        transact_compat_rx, &transaction);

    if (status == T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE) {
        uint8_t lane = session->route.input_lane;
        adapter->compat_last_valid[lane] = 1u;
        adapter->compat_last_session[lane] = *session;
        adapter->compat_last_request_id[lane] = ubus_request_id;
        adapter->compat_last_address[lane] = aligned_address;
        adapter->compat_last_data[lane] = write_data;
        adapter->compat_last_mask[lane] = byte_mask;
        receipt->status = T425_CHANNEL_COMPAT_COMMIT_SUCCESS;
    } else if (status == T425_CHANNEL_DMA_UBUS_TRANSFER_RETRY) {
        receipt->status = T425_CHANNEL_COMPAT_COMMIT_RETRY;
    } else {
        receipt->status = T425_CHANNEL_COMPAT_COMMIT_ERROR;
    }
    return status;
}

t425_channel_dma_ubus_transfer_status_t
 t425_channel_dma_memory_04_load_tx_word(
    t425_channel_dma_memory_04_t *adapter,
    uint8_t slot,
    uint32_t aligned_address,
    uint32_t request_id)
{
    t425_channel_dma_lane_t *lane;
    t425_channel_dma_ubus_transfer_status_t status;

    if (adapter == NULL || adapter->memory == NULL || adapter->bank == NULL)
        return T425_CHANNEL_DMA_UBUS_TRANSFER_INVALID;

    lane = lane_for(adapter->bank, T425_CHANNEL_DMA_TX, slot);
    if (lane == NULL || (aligned_address & 3u) != 0u ||
        lane->ptr_reg != aligned_address || lane->ptr_reg > UINT32_MAX - 4u)
        return T425_CHANNEL_DMA_UBUS_TRANSFER_INVALID;

    status = t425_channel_dma_ubus_load_tx_dbuff(
        adapter->bank, slot, aligned_address, request_id,
        transact_memory, adapter);
    if (status == T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE)
        lane->ptr_reg += 4u;

    return status;
}
