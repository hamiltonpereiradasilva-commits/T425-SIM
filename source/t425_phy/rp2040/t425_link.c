#include "t425_link.h"


static float t425_tx_clock_hz(t425_link_speed_t speed)
{
    switch (speed) {
    case T425_LINK_SPEED_5M:
        return 5000000.0f;

    case T425_LINK_SPEED_10M:
        return 10000000.0f;

    case T425_LINK_SPEED_20M:
        return 20000000.0f;
    }

    return 10000000.0f;
}

static float t425_rx_clock_hz(t425_link_speed_t speed)
{
    switch (speed) {
    case T425_LINK_SPEED_5M:
        return 10000000.0f;

    case T425_LINK_SPEED_10M:
        return 20000000.0f;

    case T425_LINK_SPEED_20M:
        return 40000000.0f;
    }

    return 20000000.0f;
}

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "t425_link.pio.h"
#include "t425_link_rx.pio.h"


/*
 * O PIO TX recebe:
 *
 *   bits 0..4  = quantidade de bits - 1
 *   bits 5..   = sequência serial LSB-first
 *
 * DATA:
 *
 *   H H D0 D1 D2 D3 D4 D5 D6 D7 L
 *
 * ACK:
 *
 *   H L
 *
 * O PIO não interpreta DATA/ACK.
 * Apenas extrai a quantidade e serializa.
 */

static uint32_t build_data_word(uint8_t data)
{
    uint32_t sequence = 0;

    /*
     * Sequência serial:
     *
     * bit 0 = H
     * bit 1 = H
     * bit 2..9 = D0..D7
     * bit 10 = L
     */
    sequence |= 1u << 0;
    sequence |= 1u << 1;
    sequence |= ((uint32_t)data & 0xffu) << 2;

    /*
     * 11 bits -> count = 10
     */
    return (10u) | (sequence << 5);
}

static uint32_t build_ack_word(void)
{
    uint32_t sequence = 0;

    /*
     * ACK:
     *
     * bit 0 = H
     * bit 1 = L
     */
    sequence |= 1u << 0;

    /*
     * 2 bits -> count = 1
     */
    return (1u) | (sequence << 5);
}

static void tx_start_word(
    t425_link_t *link,
    uint32_t word)
{
    link->tx_dma_word = word;

    dma_channel_set_read_addr(
        link->tx_dma,
        &link->tx_dma_word,
        false);

    dma_channel_set_trans_count(
        link->tx_dma,
        1,
        true);
}

bool t425_link_init(
    t425_link_t *link,
    PIO pio,
    uint sm_tx,
    uint sm_rx,
    uint tx_pin,
    uint rx_pin,
    t425_link_speed_t speed)
{
    if (!link)
        return false;

    link->pio = pio;
    link->sm_tx = sm_tx;
    link->sm_rx = sm_rx;

    link->tx_pin = tx_pin;
    link->rx_pin = rx_pin;
    link->speed = speed;

    link->tx_dma = -1;
    link->tx_word = 0;
    link->tx_dma_word = 0;
    link->rx_dma = -1;

    link->rx_buffer = 0;
    link->rx_buffer_words = 0;

    uint tx_offset = pio_add_program(
        pio,
        &t425_link_tx_program);

    uint rx_offset = pio_add_program(
        pio,
        &t425_link_rx_program);

    link->tx_offset = tx_offset;
    link->rx_offset = rx_offset;

    /*
     * --------------------------------------------------------
     * TX
     * --------------------------------------------------------
     */

    {
        pio_sm_config c =
            t425_link_tx_program_get_default_config(
                tx_offset);

        sm_config_set_out_pins(
            &c,
            tx_pin,
            1);

        /*
         * O TX desloca para a direita.
         *
         * Primeiro:
         *
         *   OUT X,5
         *
         * remove bits 0..4 (count).
         *
         * Depois os bits da sequência passam a ocupar
         * a posição LSB e são enviados pela ordem correta.
         */
        sm_config_set_out_shift(
            &c,
            true,
            false,
            32);

        float div =
            (float)clock_get_hz(clk_sys) /
            t425_tx_clock_hz(speed);

        sm_config_set_clkdiv(
            &c,
            div);

        pio_gpio_init(
            pio,
            tx_pin);

        pio_sm_set_consecutive_pindirs(
            pio,
            sm_tx,
            tx_pin,
            1,
            true);

        pio_sm_init(
            pio,
            sm_tx,
            tx_offset,
            &c);
    }

    /*
     * --------------------------------------------------------
     * RX
     * --------------------------------------------------------
     *
     * START é detectado pelo WAIT; a instrução seguinte captura TYPE.
     * O programa captura TYPE + D0..D7 + STOP (10 bits), com duas
     * fases PIO por bit para acompanhar o TX (OUT + JMP).
     *
     * O ACK é produzido pelo caminho RX -> TX.
     */

    {
        pio_sm_config c =
            t425_link_rx_program_get_default_config(
                rx_offset);

        sm_config_set_in_pins(
            &c,
            rx_pin);
        sm_config_set_jmp_pin(
            &c,
            rx_pin);

        /*
         * PUSH é explícito no programa PIO.
         * Portanto não usamos AUTOPUSH.
         */
        sm_config_set_in_shift(
            &c,
            true,
            false,
            32);

        float div =
            (float)clock_get_hz(clk_sys) /
            t425_rx_clock_hz(speed);

        sm_config_set_clkdiv(
            &c,
            div);

        pio_gpio_init(
            pio,
            rx_pin);

        pio_sm_set_consecutive_pindirs(
            pio,
            sm_rx,
            rx_pin,
            1,
            false);

        pio_sm_init(
            pio,
            sm_rx,
            rx_offset,
            &c);
    }

    return true;
}

bool t425_link_dma_init(
    t425_link_t *link,
    uint32_t *rx_buffer,
    uint32_t rx_buffer_words)
{
    if (!link ||
        !rx_buffer ||
        !rx_buffer_words)
        return false;

    link->rx_buffer = rx_buffer;
    link->rx_buffer_words = rx_buffer_words;

    link->tx_dma =
        dma_claim_unused_channel(false);

    link->rx_dma =
        dma_claim_unused_channel(false);

    if (link->tx_dma < 0 ||
        link->rx_dma < 0)
        return false;

    /*
     * --------------------------------------------------------
     * TX DMA
     * --------------------------------------------------------
     */

    {
        dma_channel_config c =
            dma_channel_get_default_config(
                link->tx_dma);

        channel_config_set_transfer_data_size(
            &c,
            DMA_SIZE_32);

        channel_config_set_read_increment(
            &c,
            false);

        channel_config_set_write_increment(
            &c,
            false);

        channel_config_set_dreq(
            &c,
            pio_get_dreq(
                link->pio,
                link->sm_tx,
                true));

        dma_channel_configure(
            link->tx_dma,
            &c,
            &link->pio->txf[link->sm_tx],
            &link->tx_dma_word,
            1,
            false);
    }

    /*
     * --------------------------------------------------------
     * RX DMA
     * --------------------------------------------------------
     */

    {
        dma_channel_config c =
            dma_channel_get_default_config(
                link->rx_dma);

        channel_config_set_transfer_data_size(
            &c,
            DMA_SIZE_32);

        channel_config_set_read_increment(
            &c,
            false);

        channel_config_set_write_increment(
            &c,
            true);

        channel_config_set_dreq(
            &c,
            pio_get_dreq(
                link->pio,
                link->sm_rx,
                false));

        dma_channel_configure(
            link->rx_dma,
            &c,
            link->rx_buffer,
            &link->pio->rxf[link->sm_rx],
            link->rx_buffer_words,
            false);
    }

    return true;
}

void t425_link_start(
    t425_link_t *link)
{
    uint mask =
        (1u << link->sm_tx) |
        (1u << link->sm_rx);

    pio_enable_sm_mask_in_sync(
        link->pio,
        mask);
}

void t425_link_stop(
    t425_link_t *link)
{
    uint mask =
        (1u << link->sm_tx) |
        (1u << link->sm_rx);

    pio_set_sm_mask_enabled(
        link->pio,
        mask,
        false);
}

void t425_link_send_frame(
    t425_link_t *link,
    uint16_t data)
{
    link->tx_word = (uint8_t)(data & 0xffu);

    tx_start_word(
        link,
        build_data_word(link->tx_word));
}

void t425_link_send_ack(
    t425_link_t *link)
{
    tx_start_word(
        link,
        build_ack_word());
}

bool t425_link_rx_available(
    t425_link_t *link)
{
    return !pio_sm_is_rx_fifo_empty(
        link->pio,
        link->sm_rx);
}

uint32_t t425_link_rx_get(
    t425_link_t *link)
{
    return pio_sm_get(
        link->pio,
        link->sm_rx);
}
