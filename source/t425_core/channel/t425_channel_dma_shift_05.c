#include "t425_channel_dma_shift_05.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>


static t425_channel_dma_lane_t *lane(
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

static const t425_channel_dma_lane_t *const_lane(
    const t425_channel_dma_bank_t *bank,
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

static uint8_t nbytes(const t425_channel_dma_lane_t *channel)
{
    return channel != NULL ? (uint8_t)(channel->shift_valid_bits / 8u) : 0u;
}

static bool count_is_negative(uint32_t count_reg)
{
    return (count_reg & 0x80000000u) != 0u;
}

static uint32_t count_length(uint32_t initial_count_reg)
{
    return 0u - initial_count_reg;
}

static bool session_progress(
    uint32_t initial_count_reg,
    uint32_t current_count_reg,
    uint32_t *progress,
    uint32_t *remaining)
{
    uint32_t total;
    uint32_t done;

    if (progress == NULL || remaining == NULL)
        return false;
    total = count_length(initial_count_reg);
    if (total != 0u && !count_is_negative(initial_count_reg))
        return false;
    done = current_count_reg - initial_count_reg;
    if (done > total)
        return false;
    if (done != total && !count_is_negative(current_count_reg))
        return false;
    if (done == total && current_count_reg != 0u)
        return false;
    *progress = done;
    *remaining = total - done;
    return true;
}

static uint32_t align_down(uint32_t address)
{
    return address & ~3u;
}

static bool align_up_checked(uint64_t address, uint32_t *aligned)
{
    uint64_t value;
    if (aligned == NULL || address > (uint64_t)UINT32_MAX + 1u)
        return false;
    value = (address + 3u) & ~(uint64_t)3u;
    if (value > (uint64_t)UINT32_MAX + 1u)
        return false;
    *aligned = (uint32_t)value;
    return true;
}

bool t425_channel_dma_shift_05_init(
    t425_channel_dma_shift_05_t *engine,
    t425_channel_dma_bank_t *bank,
    t425_channel_dma_memory_04_t *memory)
{
    if (engine == NULL || bank == NULL || memory == NULL ||
        memory->bank != bank || memory->memory == NULL)
        return false;
    memset(engine, 0, sizeof(*engine));
    engine->bank = bank;
    engine->memory = memory;
    engine->next_ticket = 1u;
    return true;
}

t425_channel_dma_05_status_t t425_channel_dma_shift_05_rx_begin(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint32_t byte_address,
    uint32_t length,
    uint32_t request_id)
{
    t425_channel_dma_lane_t *channel;
    t425_channel_dma_05_rx_state_t *state;
    uint64_t end;
    uint32_t expected_length;

    if (engine == NULL || engine->bank == NULL || slot >= 4u)
        return T425_CHANNEL_DMA_05_INVALID;
    channel = lane(engine->bank, T425_CHANNEL_DMA_RX, slot);
    state = &engine->rx[slot];
    end = (uint64_t)byte_address + length;
    if (channel == NULL)
        return T425_CHANNEL_DMA_05_INVALID;
    expected_length = count_length(channel->count_reg);

    if (state->active || nbytes(channel) != 0u ||
        channel->ptr_reg != align_down(byte_address) ||
        end > (uint64_t)UINT32_MAX - 3u ||
        (length != 0u && channel->ptr_reg > UINT32_MAX - 4u) ||
        expected_length != length ||
        (length != 0u && !count_is_negative(channel->count_reg)) ||
        (length == 0u && channel->count_reg != 0u))
        return T425_CHANNEL_DMA_05_INVALID;

    memset(state, 0, sizeof(*state));
    state->configured = true;
    state->active = length != 0u;
    state->buffer_start_address = byte_address;
    state->initial_count_reg = channel->count_reg;
    state->next_request_id = request_id;
    state->first_word_offset = (uint8_t)(byte_address & 3u);
    return T425_CHANNEL_DMA_05_OK;
}

static t425_channel_dma_05_status_t flush_rx(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    t425_channel_dma_ubus_transfer_status_t *memory_status)
{
    t425_channel_dma_lane_t *channel;
    t425_channel_dma_05_rx_state_t *state;
    t425_channel_dma_ubus_transfer_result_t result;
    uint32_t progress, remaining, pending_start;
    uint8_t count, bytes[4];

    if (engine == NULL || slot >= 4u)
        return T425_CHANNEL_DMA_05_INVALID;
    channel = lane(engine->bank, T425_CHANNEL_DMA_RX, slot);
    state = &engine->rx[slot];
    count = nbytes(channel);
    if (channel == NULL || !state->configured || count == 0u || count > 4u ||
        !session_progress(state->initial_count_reg, channel->count_reg,
                          &progress, &remaining) ||
        progress < count)
        return T425_CHANNEL_DMA_05_INVALID;

    pending_start = state->buffer_start_address + progress - count;
    if (channel->ptr_reg != align_down(pending_start))
        return T425_CHANNEL_DMA_05_ERROR;
    for (uint8_t i = 0u; i < count; ++i)
        bytes[i] = (uint8_t)(channel->shift_register >> (8u * i));

    result = t425_channel_dma_memory_04_write_rx_bytes(
        engine->memory, slot, pending_start, bytes, count,
        state->next_request_id);
    if (memory_status != NULL)
        *memory_status = result.status;
    if (result.status == T425_CHANNEL_DMA_UBUS_TRANSFER_RETRY)
        return T425_CHANNEL_DMA_05_RETRY;
    if (result.status != T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE ||
        result.bytes_completed != count || result.requests_completed != 1u)
        return T425_CHANNEL_DMA_05_ERROR;

    /* F04 commits PtrReg after the word write; F05 commits only buffer state. */
    state->next_request_id++;
    channel->shift_register = 0u;
    channel->shift_valid_bits = 0u;
    if (remaining == 0u)
        state->active = false;
    return T425_CHANNEL_DMA_05_OK;
}

t425_channel_dma_05_status_t t425_channel_dma_shift_05_rx_drain(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot)
{
    return flush_rx(engine, slot, NULL);
}

t425_channel_dma_05_rx_result_t t425_channel_dma_shift_05_rx_offer(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint8_t value)
{
    t425_channel_dma_05_rx_result_t result = {
        T425_CHANNEL_DMA_05_INVALID,
        T425_CHANNEL_DMA_UBUS_TRANSFER_INVALID,
        false
    };
    t425_channel_dma_lane_t *channel;
    t425_channel_dma_05_rx_state_t *state;
    uint32_t progress, remaining, next_byte_address;
    uint32_t pending_start;
    uint8_t count, capacity;
    t425_channel_dma_05_status_t flushed;

    if (engine == NULL || engine->bank == NULL || slot >= 4u)
        return result;
    channel = lane(engine->bank, T425_CHANNEL_DMA_RX, slot);
    state = &engine->rx[slot];
    if (channel == NULL || !state->configured || !state->active ||
        !session_progress(state->initial_count_reg, channel->count_reg,
                          &progress, &remaining) ||
        remaining == 0u)
        return result;

    count = nbytes(channel);
    if (count > 4u || progress < count)
        return result;
    pending_start = state->buffer_start_address + progress - count;
    capacity = (uint8_t)(4u - (pending_start & 3u));
    if (count >= capacity) {
        flushed = flush_rx(engine, slot, &result.memory_status);
        if (flushed == T425_CHANNEL_DMA_05_RETRY) {
            result.status = T425_CHANNEL_DMA_05_BACKPRESSURE;
            return result;
        }
        if (flushed != T425_CHANNEL_DMA_05_OK) {
            result.status = T425_CHANNEL_DMA_05_ERROR;
            return result;
        }
        count = nbytes(channel);
        if (count != 0u) {
            result.status = T425_CHANNEL_DMA_05_ERROR;
            return result;
        }
    }

    next_byte_address = state->buffer_start_address + progress;
    if (channel->ptr_reg != align_down(next_byte_address) &&
        !((next_byte_address & 3u) == 0u &&
          channel->ptr_reg == next_byte_address))
        return result;

    channel->shift_register |= (uint32_t)value << (8u * count);
    channel->shift_valid_bits = (uint8_t)((count + 1u) * 8u);
    if (!t425_channel_dma_record_rx_byte(engine->bank, slot)) {
        result.status = T425_CHANNEL_DMA_05_ERROR;
        return result;
    }
    result.input_accepted = true;
    result.status = T425_CHANNEL_DMA_05_OK;
    result.memory_status = T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE;

    count++;
    (void)session_progress(state->initial_count_reg, channel->count_reg,
                           &progress, &remaining);
    pending_start = state->buffer_start_address + progress - count;
    capacity = (uint8_t)(4u - (pending_start & 3u));
    if (count == capacity || remaining == 0u) {
        flushed = flush_rx(engine, slot, &result.memory_status);
        if (flushed == T425_CHANNEL_DMA_05_RETRY)
            result.status = T425_CHANNEL_DMA_05_RETRY;
        else if (flushed != T425_CHANNEL_DMA_05_OK)
            result.status = T425_CHANNEL_DMA_05_ERROR;
    }
    return result;
}

bool t425_channel_dma_shift_05_rx_complete(
    const t425_channel_dma_shift_05_t *engine,
    uint8_t slot)
{
    const t425_channel_dma_lane_t *channel;
    const t425_channel_dma_05_rx_state_t *state;
    uint32_t progress, remaining;
    if (engine == NULL || engine->bank == NULL || slot >= 4u)
        return false;
    channel = const_lane(engine->bank, T425_CHANNEL_DMA_RX, slot);
    state = &engine->rx[slot];
    return channel != NULL && state->configured && !state->active &&
           session_progress(state->initial_count_reg, channel->count_reg,
                            &progress, &remaining) &&
           remaining == 0u && nbytes(channel) == 0u;
}

t425_channel_dma_05_status_t t425_channel_dma_shift_05_tx_begin(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint32_t byte_address,
    uint32_t length,
    uint32_t request_id)
{
    t425_channel_dma_lane_t *channel;
    t425_channel_dma_05_tx_state_t *state;
    uint32_t aligned_address;
    uint64_t end;
    uint32_t expected_length;

    if (engine == NULL || engine->bank == NULL || slot >= 4u)
        return T425_CHANNEL_DMA_05_INVALID;
    channel = lane(engine->bank, T425_CHANNEL_DMA_TX, slot);
    state = &engine->tx[slot];
    aligned_address = align_down(byte_address);
    end = (uint64_t)byte_address + length;
    if (channel == NULL)
        return T425_CHANNEL_DMA_05_INVALID;
    expected_length = count_length(channel->count_reg);

    if (state->active ||
        state->stage != T425_CHANNEL_DMA_05_TX_IDLE || nbytes(channel) != 0u ||
        channel->ptr_reg != aligned_address ||
        end > (uint64_t)UINT32_MAX - 3u ||
        (length != 0u && channel->ptr_reg > UINT32_MAX - 4u) ||
        expected_length != length ||
        (length != 0u && !count_is_negative(channel->count_reg)) ||
        (length == 0u && channel->count_reg != 0u))
        return T425_CHANNEL_DMA_05_INVALID;

    memset(state, 0, sizeof(*state));
    state->configured = true;
    state->active = length != 0u;
    state->buffer_start_address = byte_address;
    state->initial_count_reg = channel->count_reg;
    state->next_request_id = request_id;
    state->first_word_offset = (uint8_t)(byte_address & 3u);
    state->stage = T425_CHANNEL_DMA_05_TX_IDLE;
    return T425_CHANNEL_DMA_05_OK;
}

static uint64_t next_ticket(t425_channel_dma_shift_05_t *engine)
{
    uint64_t ticket = engine->next_ticket++;
    if (ticket == 0u)
        ticket = engine->next_ticket++;
    return ticket;
}

t425_channel_dma_05_status_t t425_channel_dma_shift_05_tx_prepare(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint8_t *offered_byte,
    uint64_t *ticket)
{
    t425_channel_dma_lane_t *channel;
    t425_channel_dma_05_tx_state_t *state;
    uint32_t progress, remaining;
    uint8_t count;

    if (engine == NULL || engine->bank == NULL || engine->memory == NULL ||
        slot >= 4u || offered_byte == NULL || ticket == NULL)
        return T425_CHANNEL_DMA_05_INVALID;
    channel = lane(engine->bank, T425_CHANNEL_DMA_TX, slot);
    state = &engine->tx[slot];
    if (state->submission_reserved)
        return T425_CHANNEL_DMA_05_BUSY;
    if (state->stage == T425_CHANNEL_DMA_05_TX_OFFERED) {
        if (!t425_channel_dma_shift_05_invariants(
                engine, T425_CHANNEL_DMA_TX, slot))
            return T425_CHANNEL_DMA_05_ERROR;
        *offered_byte = state->offered_byte;
        *ticket = state->ticket;
        return T425_CHANNEL_DMA_05_OK;
    }
    if (state->stage != T425_CHANNEL_DMA_05_TX_IDLE)
        return T425_CHANNEL_DMA_05_BUSY;
    if (!state->configured || !state->active ||
        !session_progress(state->initial_count_reg, channel->count_reg,
                          &progress, &remaining) ||
        remaining == 0u)
        return T425_CHANNEL_DMA_05_INVALID;

    count = nbytes(channel);
    if (count == 0u) {
        uint8_t offset = state->first_word_loaded ? 0u :
                         state->first_word_offset;
        t425_channel_dma_ubus_transfer_status_t memory_status =
            t425_channel_dma_memory_04_load_tx_word(
                engine->memory, slot, channel->ptr_reg,
                state->next_request_id);
        if (memory_status == T425_CHANNEL_DMA_UBUS_TRANSFER_RETRY)
            return T425_CHANNEL_DMA_05_RETRY;
        if (memory_status != T425_CHANNEL_DMA_UBUS_TRANSFER_COMPLETE)
            return T425_CHANNEL_DMA_05_ERROR;

        state->next_request_id++;
        state->first_word_loaded = true;
        count = (uint8_t)(4u - offset);
        if (count > remaining)
            count = (uint8_t)remaining;
        channel->shift_register = channel->dbuff_reg >> (8u * offset);
        channel->shift_valid_bits = (uint8_t)(count * 8u);
    }
    if (count == 0u || count > 4u ||
        (channel->shift_valid_bits & 7u) != 0u)
        return T425_CHANNEL_DMA_05_ERROR;

    state->offered_byte = (uint8_t)channel->shift_register;
    state->ticket = next_ticket(engine);
    state->stage = T425_CHANNEL_DMA_05_TX_OFFERED;
    *offered_byte = state->offered_byte;
    *ticket = state->ticket;
    { uint32_t source=state->buffer_start_address+progress; uint32_t word_addr=source&~3u;
      uint8_t source_byte=(uint8_t)(channel->dbuff_reg >> (8u*(source&3u)));
      {} }
    return T425_CHANNEL_DMA_05_OK;
}

bool t425_channel_dma_shift_05_tx_submitted(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint64_t ticket)
{
    t425_channel_dma_05_tx_state_t *state;
    if (engine == NULL || engine->bank == NULL || slot >= 4u || ticket == 0u ||
        !t425_channel_dma_shift_05_invariants(
            engine, T425_CHANNEL_DMA_TX, slot))
        return false;
    state = &engine->tx[slot];
    if (!state->active || state->ticket != ticket ||
        state->stage != T425_CHANNEL_DMA_05_TX_OFFERED ||
        state->submission_reserved)
        return false;
    state->stage = T425_CHANNEL_DMA_05_TX_SUBMITTED;
    return true;
}

bool t425_channel_dma_shift_05_tx_submission_preflight(
    const t425_channel_dma_shift_05_t *engine,
    uint8_t slot, uint64_t ticket, uint8_t byte)
{
    const t425_channel_dma_05_tx_state_t *state;
    const t425_channel_dma_lane_t *channel;
    if (engine == NULL || engine->bank == NULL || slot >= 4u || ticket == 0u ||
        !t425_channel_dma_shift_05_invariants(
            engine, T425_CHANNEL_DMA_TX, slot))
        return false;
    state = &engine->tx[slot];
    channel = const_lane(engine->bank, T425_CHANNEL_DMA_TX, slot);
    return channel != NULL && state->active && !state->submission_reserved &&
        state->ticket == ticket && state->offered_byte == byte &&
        state->stage == T425_CHANNEL_DMA_05_TX_OFFERED;
}

bool t425_channel_dma_shift_05_tx_submission_reserve(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot, uint64_t ticket, uint8_t byte,
    t425_channel_dma_05_tx_reservation_t *reservation)
{
    t425_channel_dma_05_tx_state_t *state;
    if (reservation == NULL ||
        !t425_channel_dma_shift_05_tx_submission_preflight(
            engine, slot, ticket, byte))
        return false;
    state = &engine->tx[slot];
    ++engine->next_submission_cookie;
    if (engine->next_submission_cookie == 0u)
        ++engine->next_submission_cookie;
    state->submission_reserved = true;
    state->submission_cookie = engine->next_submission_cookie;
    *reservation = (t425_channel_dma_05_tx_reservation_t){
        engine, slot, ticket, state->submission_cookie, true};
    return true;
}

bool t425_channel_dma_shift_05_tx_submission_reservation_valid(
    const t425_channel_dma_05_tx_reservation_t *reservation)
{
    const t425_channel_dma_05_tx_state_t *state;
    if (reservation == NULL || !reservation->valid ||
        reservation->engine == NULL || reservation->slot >= 4u ||
        reservation->ticket == 0u)
        return false;
    state = &reservation->engine->tx[reservation->slot];
    return state->submission_reserved && state->submission_cookie ==
        reservation->cookie && state->active &&
        state->ticket == reservation->ticket &&
        state->stage == T425_CHANNEL_DMA_05_TX_OFFERED;
}

void t425_channel_dma_shift_05_tx_submission_cancel(
    t425_channel_dma_05_tx_reservation_t *reservation)
{
    t425_channel_dma_05_tx_state_t *state;
    if (!t425_channel_dma_shift_05_tx_submission_reservation_valid(
            reservation))
        return;
    state = &reservation->engine->tx[reservation->slot];
    state->submission_reserved = false;
    state->submission_cookie = 0u;
    reservation->valid = false;
}

void t425_channel_dma_shift_05_tx_submission_commit(
    t425_channel_dma_05_tx_reservation_t *reservation)
{
    t425_channel_dma_05_tx_state_t *state;
    /* Precondition: coordinator validated the token; Core0 exclusivity
     * prevents mutation between preflight and this local commit. */
    state = &reservation->engine->tx[reservation->slot];
    state->stage = T425_CHANNEL_DMA_05_TX_SUBMITTED;
    state->submission_reserved = false;
    state->submission_cookie = 0u;
    reservation->valid = false;
}

bool t425_channel_dma_shift_05_tx_physical_complete(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint64_t ticket)
{
    t425_channel_dma_05_tx_state_t *state;
    if (engine == NULL || engine->bank == NULL || slot >= 4u || ticket == 0u ||
        !t425_channel_dma_shift_05_invariants(
            engine, T425_CHANNEL_DMA_TX, slot))
        return false;
    state = &engine->tx[slot];
    if (!state->active || state->ticket != ticket ||
        state->stage != T425_CHANNEL_DMA_05_TX_SUBMITTED)
        return false;
    state->stage = T425_CHANNEL_DMA_05_TX_PHYSICALLY_COMPLETE;
    return true;
}

bool t425_channel_dma_shift_05_tx_ack(
    t425_channel_dma_shift_05_t *engine,
    uint8_t slot,
    uint64_t ticket)
{
    t425_channel_dma_lane_t *channel;
    t425_channel_dma_05_tx_state_t *state;
    uint32_t progress, remaining;
    uint8_t count;

    if (engine == NULL || engine->bank == NULL || slot >= 4u || ticket == 0u)
        return false;
    channel = lane(engine->bank, T425_CHANNEL_DMA_TX, slot);
    state = &engine->tx[slot];
    count = nbytes(channel);
    if (channel == NULL || !state->configured || !state->active ||
        state->ticket != ticket ||
        state->stage != T425_CHANNEL_DMA_05_TX_PHYSICALLY_COMPLETE ||
        count == 0u ||
        !session_progress(state->initial_count_reg, channel->count_reg,
                          &progress, &remaining) ||
        remaining == 0u)
        return false;

    if (remaining == 1u && count != 1u)
        return false;
    if (!t425_channel_dma_record_tx_ack(engine->bank, slot))
        return false;
    channel->shift_register >>= 8u;
    channel->shift_valid_bits = (uint8_t)(channel->shift_valid_bits - 8u);
    state->ticket = 0u;
    state->stage = T425_CHANNEL_DMA_05_TX_IDLE;
    state->offered_byte = 0u;
    (void)session_progress(state->initial_count_reg, channel->count_reg,
                           &progress, &remaining);
    if (remaining == 0u)
        state->active = false;
    return true;
}

bool t425_channel_dma_shift_05_tx_complete(
    const t425_channel_dma_shift_05_t *engine,
    uint8_t slot)
{
    const t425_channel_dma_lane_t *channel;
    const t425_channel_dma_05_tx_state_t *state;
    uint32_t progress, remaining;
    if (engine == NULL || engine->bank == NULL || slot >= 4u)
        return false;
    channel = const_lane(engine->bank, T425_CHANNEL_DMA_TX, slot);
    state = &engine->tx[slot];
    return channel != NULL && state->configured && !state->active &&
           state->stage == T425_CHANNEL_DMA_05_TX_IDLE &&
           state->ticket == 0u &&
           session_progress(state->initial_count_reg, channel->count_reg,
                            &progress, &remaining) &&
           remaining == 0u && nbytes(channel) == 0u;
}

bool t425_channel_dma_shift_05_invariants(
    const t425_channel_dma_shift_05_t *engine,
    t425_channel_dma_direction_t direction,
    uint8_t slot)
{
    const t425_channel_dma_lane_t *channel;
    uint32_t progress, remaining, expected_pointer;
    uint8_t count;

    if (engine == NULL || engine->bank == NULL || slot >= 4u)
        return false;
    channel = const_lane(engine->bank, direction, slot);
    if (channel == NULL || channel->shift_valid_bits > 32u ||
        (channel->shift_valid_bits & 7u) != 0u)
        return false;
    count = nbytes(channel);

    if (direction == T425_CHANNEL_DMA_RX) {
        const t425_channel_dma_05_rx_state_t *state = &engine->rx[slot];
        uint64_t endpoint;
        uint32_t pending_start;
        if (!state->configured)
            return !state->active && count == 0u;
        if (state->first_word_offset > 3u ||
            state->first_word_offset !=
                (uint8_t)(state->buffer_start_address & 3u) ||
            !session_progress(state->initial_count_reg, channel->count_reg,
                              &progress, &remaining) ||
            progress < count || (remaining == 0u && state->active && count == 0u))
            return false;
        if (count != 0u) {
            pending_start = state->buffer_start_address + progress - count;
            if (channel->ptr_reg != align_down(pending_start))
                return false;
        } else if (progress == 0u) {
            if (channel->ptr_reg != align_down(state->buffer_start_address))
                return false;
        } else {
            endpoint = (uint64_t)state->buffer_start_address + progress;
            if (!align_up_checked(endpoint, &expected_pointer) ||
                channel->ptr_reg != expected_pointer)
                return false;
        }
        return true;
    }

    if (direction == T425_CHANNEL_DMA_TX) {
        const t425_channel_dma_05_tx_state_t *state = &engine->tx[slot];
        uint64_t loaded_end;
        if (!state->configured) {
            return !state->active && count == 0u &&
                   state->stage == T425_CHANNEL_DMA_05_TX_IDLE &&
                   state->ticket == 0u;
        }
        if (!session_progress(state->initial_count_reg, channel->count_reg,
                              &progress, &remaining) ||
            (uint64_t)progress + count > count_length(state->initial_count_reg) ||
            (remaining == 0u && (state->active || count != 0u)) ||
            state->first_word_offset > 3u ||
            state->stage > T425_CHANNEL_DMA_05_TX_PHYSICALLY_COMPLETE ||
            ((state->stage == T425_CHANNEL_DMA_05_TX_IDLE) !=
             (state->ticket == 0u)) ||
            (state->stage != T425_CHANNEL_DMA_05_TX_IDLE &&
             (!state->active || state->offered_byte !=
              (uint8_t)channel->shift_register)))
            return false;
        if (!state->first_word_loaded) {
            return channel->ptr_reg == align_down(state->buffer_start_address);
        }
        loaded_end = (uint64_t)state->buffer_start_address + progress + count;
        if (!align_up_checked(loaded_end, &expected_pointer))
            return false;
        return channel->ptr_reg == expected_pointer;
    }
    return false;
}

t425_channel_dma_05_status_t t425_channel_dma_shift_05_try_abort(
    t425_channel_dma_shift_05_t *engine,
    t425_channel_dma_direction_t direction,
    uint8_t slot)
{
    t425_channel_dma_lane_t *channel;
    if (engine == NULL || slot >= 4u)
        return T425_CHANNEL_DMA_05_INVALID;
    channel = lane(engine->bank, direction, slot);
    if (channel == NULL)
        return T425_CHANNEL_DMA_05_INVALID;

    if (direction == T425_CHANNEL_DMA_RX) {
        t425_channel_dma_05_rx_state_t *state = &engine->rx[slot];
        if (state->active || nbytes(channel) != 0u)
            return T425_CHANNEL_DMA_05_BUSY;
        memset(state, 0, sizeof(*state));
        return T425_CHANNEL_DMA_05_OK;
    }
    if (direction == T425_CHANNEL_DMA_TX) {
        t425_channel_dma_05_tx_state_t *state = &engine->tx[slot];
        if (state->active || nbytes(channel) != 0u ||
            state->stage != T425_CHANNEL_DMA_05_TX_IDLE ||
            state->ticket != 0u)
            return T425_CHANNEL_DMA_05_BUSY;
        memset(state, 0, sizeof(*state));
        return T425_CHANNEL_DMA_05_OK;
    }
    return T425_CHANNEL_DMA_05_INVALID;
}
