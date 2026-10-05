#include "t425_core_phy_rp2040.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"

static t425_core_phy_rp2040_t *ack_irq_adapter;

static bool valid_adapter(const t425_core_phy_rp2040_t *adapter)
{
    if (adapter == 0 || adapter->route_count != T425_CORE_PHY_ENDPOINT_COUNT)
        return false;

    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        if (adapter->links[i] == 0)
            return false;
    }
    return true;
}

static t425_link_t *resolve_endpoint(
    t425_core_phy_rp2040_t *adapter,
    uint32_t endpoint_id)
{
    if (!valid_adapter(adapter))
        return 0;

    for (uint32_t i = 0u; i < adapter->route_count; ++i) {
        const t425_core_phy_route_t *route = &adapter->routes[i];
        if (route->endpoint_id == endpoint_id &&
            route->physical_link_id < T425_CORE_PHY_PHYSICAL_LINK_COUNT)
            return adapter->links[route->physical_link_id];
    }
    return 0;
}

static uint32_t physical_index_for_endpoint(
    const t425_core_phy_rp2040_t *adapter,
    uint32_t endpoint_id)
{
    for (uint32_t i = 0u; i < adapter->route_count; ++i) {
        if (adapter->routes[i].endpoint_id == endpoint_id)
            return adapter->routes[i].physical_link_id;
    }
    return T425_CORE_PHY_PHYSICAL_LINK_COUNT;
}

/*
 * RX word layout with shift_right=true:
 *
 *   ISR[31]    = START
 *   ISR[30]    = TYPE
 *   ISR[29:22] = D0..D7
 *   ISR[21]    = STOP
 *
 * DATA:
 *   START=1, TYPE=1, STOP=0
 *
 * ACK:
 *   START=1, TYPE=0
 *
 * The PIO RX program is responsible only for sampling and pushing
 * the physical word. This adapter decodes the resulting ISR word.
 */
#define T425_RX_START_BIT   31u
#define T425_RX_TYPE_BIT    30u
#define T425_RX_DATA_SHIFT  22u
#define T425_RX_STOP_BIT    21u
#define T425_PHY_LINK_SPECIAL_GPIO       20u
#define T425_PHY_LINK0_SPECIAL_GPIO      21u
#define T425_PHY_LINK123_SPECIAL_GPIO    22u
#define T425_PHY_RESET_GPIO               4u
#define T425_PHY_ANALYSE_GPIO             5u
#define T425_PHY_ERROR_IN_GPIO            6u
#define T425_PHY_BOOT_FROM_ROM_GPIO       7u
#define T425_PHY_EVENT_REQ_GPIO           8u
#define T425_PHY_DISABLE_INT_RAM_GPIO     9u
#define T425_PHY_PROC_SPEED0_GPIO        10u
#define T425_PHY_PROC_SPEED1_GPIO        11u
#define T425_PHY_PROC_SPEED2_GPIO        12u
#define T425_PHY_EVENT_ERROR_GPIO        13u
#define T425_PHY_EVENT_ACK_GPIO          14u
#define T425_PHY_EVENT_WAITING_GPIO      15u

#define T425_RX_ACK_WORD    0x80000000u

static void t425_phy_speed_gpio_init(void)
{
    gpio_init(T425_PHY_LINK_SPECIAL_GPIO);
    gpio_set_dir(T425_PHY_LINK_SPECIAL_GPIO, GPIO_IN);

    gpio_init(T425_PHY_LINK0_SPECIAL_GPIO);
    gpio_set_dir(T425_PHY_LINK0_SPECIAL_GPIO, GPIO_IN);

    gpio_init(T425_PHY_LINK123_SPECIAL_GPIO);
    gpio_set_dir(T425_PHY_LINK123_SPECIAL_GPIO, GPIO_IN);
}

static void t425_phy_system_gpio_init(void)
{
    const uint inputs[] = {
        T425_PHY_RESET_GPIO,
        T425_PHY_ANALYSE_GPIO,
        T425_PHY_ERROR_IN_GPIO,
        T425_PHY_BOOT_FROM_ROM_GPIO,
        T425_PHY_EVENT_REQ_GPIO,
        T425_PHY_DISABLE_INT_RAM_GPIO,
        T425_PHY_PROC_SPEED0_GPIO,
        T425_PHY_PROC_SPEED1_GPIO,
        T425_PHY_PROC_SPEED2_GPIO
    };
    uint i;

    for (i = 0u; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        gpio_init(inputs[i]);
        gpio_set_dir(inputs[i], GPIO_IN);
    }

    gpio_init(T425_PHY_EVENT_ERROR_GPIO);
    gpio_set_dir(T425_PHY_EVENT_ERROR_GPIO, GPIO_OUT);
    gpio_init(T425_PHY_EVENT_ACK_GPIO);
    gpio_set_dir(T425_PHY_EVENT_ACK_GPIO, GPIO_OUT);
    gpio_init(T425_PHY_EVENT_WAITING_GPIO);
    gpio_set_dir(T425_PHY_EVENT_WAITING_GPIO, GPIO_OUT);
    gpio_put(T425_PHY_EVENT_ERROR_GPIO, false);
    gpio_put(T425_PHY_EVENT_ACK_GPIO, false);
    gpio_put(T425_PHY_EVENT_WAITING_GPIO, false);
}

static bool rp2040_read_system_inputs(
    void *context,
    t425_core_phy_system_inputs_t *inputs)
{
    (void)context;
    if (inputs == 0)
        return false;

    inputs->reset = gpio_get(T425_PHY_RESET_GPIO);
    inputs->analyse = gpio_get(T425_PHY_ANALYSE_GPIO);
    inputs->error_in = gpio_get(T425_PHY_ERROR_IN_GPIO);
    inputs->boot_from_rom = gpio_get(T425_PHY_BOOT_FROM_ROM_GPIO);
    inputs->event_req = gpio_get(T425_PHY_EVENT_REQ_GPIO);
    inputs->disable_int_ram = gpio_get(T425_PHY_DISABLE_INT_RAM_GPIO);
    inputs->proc_speed_select[0] = gpio_get(T425_PHY_PROC_SPEED0_GPIO);
    inputs->proc_speed_select[1] = gpio_get(T425_PHY_PROC_SPEED1_GPIO);
    inputs->proc_speed_select[2] = gpio_get(T425_PHY_PROC_SPEED2_GPIO);
    inputs->link_special = gpio_get(T425_PHY_LINK_SPECIAL_GPIO);
    inputs->link0_special = gpio_get(T425_PHY_LINK0_SPECIAL_GPIO);
    inputs->link123_special = gpio_get(T425_PHY_LINK123_SPECIAL_GPIO);
    return true;
}

static bool rp2040_write_system_outputs(
    void *context,
    const t425_core_phy_system_outputs_t *outputs)
{
    (void)context;
    if (outputs == 0)
        return false;

    gpio_put(T425_PHY_EVENT_ERROR_GPIO, outputs->error);
    gpio_put(T425_PHY_EVENT_ACK_GPIO, outputs->event_ack);
    gpio_put(T425_PHY_EVENT_WAITING_GPIO, outputs->event_waiting);
    return true;
}

static t425_link_speed_t t425_phy_decode_speed(
    bool link_special,
    bool group_special)
{
    if (!link_special && !group_special)
        return T425_LINK_SPEED_10M;

    if (!link_special && group_special)
        return T425_LINK_SPEED_5M;

    if (link_special && !group_special)
        return T425_LINK_SPEED_10M;

    return T425_LINK_SPEED_20M;
}

static void pump_rx(t425_core_phy_rp2040_t *adapter, uint32_t physical_index)
{
    t425_link_t *link = adapter->links[physical_index];

    while (!adapter->rx_data_pending[physical_index] &&
           t425_link_rx_available(link)) {

        uint32_t raw = t425_link_rx_get(link);

        uint32_t start = (raw >> T425_RX_START_BIT) & 1u;
        uint32_t type  = (raw >> T425_RX_TYPE_BIT) & 1u;
        uint32_t stop  = (raw >> T425_RX_STOP_BIT) & 1u;

        /*
         * DATA:
         *
         *   START = 1
         *   TYPE  = 1
         *   DATA  = ISR[29:22]
         *   STOP  = 0
         */
        if (start && type && !stop) {

            adapter->rx_data[physical_index] =
                (uint8_t)((raw >> T425_RX_DATA_SHIFT) & 0xFFu);

            adapter->rx_data_pending[physical_index] = true;
        }

        /*
         * ACK:
         *
         *   START = 1
         *   TYPE  = 0
         *
         * With shift_right=true and ISR cleared before reception,
         * the resulting ACK word is 0x80000000.
         */
        else if (raw == T425_RX_ACK_WORD) {

            adapter->physical_ack_count[physical_index]++;
            t425_link_note_ack_received(link);
        }

        /*
         * Other words are malformed/control words and remain PHY-local.
         */

        if (adapter->physical_ack_pending[physical_index] &&
            !pio_sm_is_rx_fifo_full(link->pio, link->sm_rx) &&
            t425_link_send_ack(link)) {

            adapter->physical_ack_pending[physical_index] = false;
        }
    }
}

/* Called from the PIO IRQ on Core1 as soon as the RX PIO sees DATA TYPE. */
static void service_ack_flags(PIO pio)
{
    t425_core_phy_rp2040_t *adapter = ack_irq_adapter;
    if (!valid_adapter(adapter))
        return;

    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        t425_link_t *link = adapter->links[i];
        if (link->pio != pio || link->sm_rx >= 4u)
            continue;
        if (pio_interrupt_get(pio, link->sm_rx)) {
            pio_interrupt_clear(pio, link->sm_rx);
            /*
             * ACK once the DATA packet is identified if the RX FIFO can
             * accept it and TX is idle. Otherwise defer in PHY state.
             */
            if (pio_sm_is_rx_fifo_full(pio, link->sm_rx) ||
                !t425_link_send_ack(link))
                adapter->physical_ack_pending[i] = true;
        }
    }
}

static void pio0_irq_handler(void) { service_ack_flags(pio0); }
static void pio1_irq_handler(void) { service_ack_flags(pio1); }

static bool register_ack_irqs(t425_core_phy_rp2040_t *adapter)
{
    if (ack_irq_adapter != 0 && ack_irq_adapter != adapter)
        return false;
    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        t425_link_t *link = adapter->links[i];
        if ((link->pio != pio0 && link->pio != pio1) ||
            link->sm_rx >= 4u || link->tx_dma < 0)
            return false;
        for (uint32_t j = 0u; j < i; ++j) {
            if (adapter->links[j]->pio == link->pio &&
                adapter->links[j]->sm_rx == link->sm_rx)
                return false;
        }
    }
    ack_irq_adapter = adapter;

    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        t425_link_t *link = adapter->links[i];
        pio_interrupt_clear(link->pio, link->sm_rx);
        pio_set_irq0_source_enabled(
            link->pio,
            (pio_interrupt_source_t)(pis_interrupt0 + link->sm_rx),
            true);
    }

    irq_set_exclusive_handler(pio_get_irq_num(pio0, 0u), pio0_irq_handler);
    irq_set_exclusive_handler(pio_get_irq_num(pio1, 0u), pio1_irq_handler);
    irq_set_enabled(pio_get_irq_num(pio0, 0u), true);
    irq_set_enabled(pio_get_irq_num(pio1, 0u), true);
    adapter->ack_irq_registered = true;
    return true;
}

static int rp2040_tx(void *context, uint32_t endpoint_id, uint16_t frame)
{
    t425_link_t *link = resolve_endpoint(context, endpoint_id);
    if (link == 0 || frame > 0xFFu)
        return -1;

    return t425_link_send_frame(link, frame) ? 0 : -2;
}

static bool rp2040_rx_available(void *context, uint32_t endpoint_id)
{
    t425_core_phy_rp2040_t *adapter = context;
    uint32_t physical_index;
    if (!valid_adapter(adapter) ||
        endpoint_id >= T425_CORE_PHY_ENDPOINT_COUNT)
        return false;
    physical_index = physical_index_for_endpoint(adapter, endpoint_id);
    if (physical_index >= T425_CORE_PHY_PHYSICAL_LINK_COUNT)
        return false;
    pump_rx(adapter, physical_index);
    return adapter->rx_data_pending[physical_index];
}

static int rp2040_rx_get(
    void *context,
    uint32_t endpoint_id,
    uint32_t *frame)
{
    t425_core_phy_rp2040_t *adapter = context;
    uint32_t physical_index;
    if (!valid_adapter(adapter) || frame == 0 ||
        endpoint_id >= T425_CORE_PHY_ENDPOINT_COUNT)
        return -1;
    physical_index = physical_index_for_endpoint(adapter, endpoint_id);
    if (physical_index >= T425_CORE_PHY_PHYSICAL_LINK_COUNT)
        return -1;
    pump_rx(adapter, physical_index);
    if (!adapter->rx_data_pending[physical_index])
        return -2;

    *frame = adapter->rx_data[physical_index];
    adapter->rx_data_pending[physical_index] = false;
    return 0;
}

bool t425_core_phy_rp2040_init(
    t425_core_phy_rp2040_t *adapter,
    t425_link_t *const links[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    const t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT])
{
    if (adapter == 0 || links == 0 || routes == 0)
        return false;

    adapter->route_count = 0u;
    adapter->ack_irq_registered = false;
    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        if (links[i] == 0 ||
            (links[i]->pio != pio0 && links[i]->pio != pio1) ||
            links[i]->sm_rx >= 4u)
            return false;
        for (uint32_t j = 0u; j < i; ++j) {
            if (links[j]->pio == links[i]->pio &&
                links[j]->sm_rx == links[i]->sm_rx)
                return false;
        }
        adapter->links[i] = links[i];
    }

    /* Require a complete bijection: four unique endpoints and four links. */
    for (uint32_t i = 0u; i < T425_CORE_PHY_ENDPOINT_COUNT; ++i) {
        if (routes[i].endpoint_id >= T425_CORE_PHY_ENDPOINT_COUNT ||
            routes[i].physical_link_id >= T425_CORE_PHY_PHYSICAL_LINK_COUNT)
            return false;
        for (uint32_t j = 0u; j < i; ++j) {
            if (routes[i].endpoint_id == routes[j].endpoint_id ||
                routes[i].physical_link_id == routes[j].physical_link_id)
                return false;
        }
        adapter->routes[i] = routes[i];
    }

    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        adapter->rx_data[i] = 0u;
        adapter->physical_ack_count[i] = 0u;
        adapter->rx_data_pending[i] = false;
        adapter->physical_ack_pending[i] = false;
    }
    adapter->route_count = T425_CORE_PHY_ENDPOINT_COUNT;
    return true;
}

bool t425_core_phy_rp2040_init_four_links(
    t425_core_phy_rp2040_t *adapter,
    t425_link_t links[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    uint32_t *const rx_buffers[T425_CORE_PHY_PHYSICAL_LINK_COUNT],
    uint32_t rx_buffer_words,
    const t425_core_phy_route_t routes[T425_CORE_PHY_ENDPOINT_COUNT])
{
    t425_link_t *link_ptrs[T425_CORE_PHY_PHYSICAL_LINK_COUNT];
    uint32_t initialized_count = 0u;
    t425_link_speed_t speed0;
    t425_link_speed_t speed123;

    if (adapter == 0 || links == 0 || rx_buffers == 0 ||
        rx_buffer_words == 0u || routes == 0)
        return false;

    t425_phy_speed_gpio_init();
    t425_phy_system_gpio_init();

    speed0 = t425_phy_decode_speed(
        gpio_get(T425_PHY_LINK_SPECIAL_GPIO),
        gpio_get(T425_PHY_LINK0_SPECIAL_GPIO));

    speed123 = t425_phy_decode_speed(
        gpio_get(T425_PHY_LINK_SPECIAL_GPIO),
        gpio_get(T425_PHY_LINK123_SPECIAL_GPIO));

    for (uint32_t physical = 0u;
         physical < T425_CORE_PHY_PHYSICAL_LINK_COUNT;
         ++physical) {
        uint32_t pair = physical & 1u;
        PIO pio = physical < 2u ? pio0 : pio1;
        uint sm_tx = pair * 2u;
        uint sm_rx = sm_tx + 1u;
        uint tx_pin;
        uint rx_pin;

        /*
         * Link2/Link3 use the board mirrored connector placement:
         * numeric GPIO order is RX/TX while semantic roles remain TX/RX.
         */
        switch (physical) {
        case 0u:
            tx_pin = 0u;
            rx_pin = 1u;
            break;

        case 1u:
            tx_pin = 2u;
            rx_pin = 3u;
            break;

        case 2u:
            tx_pin = 17u;
            rx_pin = 16u;
            break;

        case 3u:
            tx_pin = 19u;
            rx_pin = 18u;
            break;

        default:
            goto fail;
        }

        if (rx_buffers[physical] == 0 ||
            !t425_link_init(
                &links[physical],
                pio,
                sm_tx,
                sm_rx,
                tx_pin,
                rx_pin,
                physical == 0u ? speed0 : speed123))
            goto fail;
        if (!t425_link_dma_init(
                &links[physical], rx_buffers[physical], rx_buffer_words))
            goto fail;
        link_ptrs[physical] = &links[physical];
        ++initialized_count;
    }

    if (t425_core_phy_rp2040_init(adapter, link_ptrs, routes))
        return true;

fail:
    for (uint32_t i = 0u; i < initialized_count; ++i)
        t425_link_dma_deinit(&links[i]);
    return false;
}

void t425_core_phy_rp2040_make_ops(
    t425_core_phy_rp2040_t *adapter,
    t425_core_phy_ops_t *ops)
{
    if (adapter == 0 || ops == 0)
        return;

    ops->tx_frame = rp2040_tx;
    ops->rx_available = rp2040_rx_available;
    ops->rx_get = rp2040_rx_get;
    ops->read_system_inputs = rp2040_read_system_inputs;
    ops->write_system_outputs = rp2040_write_system_outputs;
    ops->context = adapter;
}

void t425_core_phy_rp2040_service(t425_core_phy_rp2040_t *adapter)
{
    if (!valid_adapter(adapter))
        return;

    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        t425_link_t *link = adapter->links[i];
        if (!adapter->physical_ack_pending[i] ||
            pio_sm_is_rx_fifo_full(link->pio, link->sm_rx))
            continue;
        if (t425_link_send_ack(link))
            adapter->physical_ack_pending[i] = false;
    }
}

bool t425_core_phy_rp2040_start(t425_core_phy_rp2040_t *adapter)
{
    if (!valid_adapter(adapter) || !register_ack_irqs(adapter))
        return false;
    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i)
        t425_link_start(adapter->links[i]);
    return true;
}

void t425_core_phy_rp2040_stop(t425_core_phy_rp2040_t *adapter)
{
    if (adapter == 0)
        return;
    for (uint32_t i = 0u; i < T425_CORE_PHY_PHYSICAL_LINK_COUNT; ++i) {
        if (adapter->links[i] != 0) {
            pio_set_irq0_source_enabled(
                adapter->links[i]->pio,
                (pio_interrupt_source_t)(pis_interrupt0 +
                                         adapter->links[i]->sm_rx),
                false);
            t425_link_stop(adapter->links[i]);
        }
        adapter->physical_ack_pending[i] = false;
        adapter->physical_ack_count[i] = 0u;
    }
    if (adapter->ack_irq_registered) {
        irq_set_enabled(pio_get_irq_num(pio0, 0u), false);
        irq_set_enabled(pio_get_irq_num(pio1, 0u), false);
        adapter->ack_irq_registered = false;
    }
}
