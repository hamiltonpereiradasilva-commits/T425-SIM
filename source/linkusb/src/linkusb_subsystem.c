#include "linkusb_subsystem.h"
#include "hardware/gpio.h"

const linkusb_subsystem_pin_config_t linkusb_subsystem_pin_map[LINKUSB_SUBSYSTEM_COUNT][LINKUSB_SUBSYSTEM_SIGNAL_COUNT] = {
    {{4u, LINKUSB_GPIO_PULL_UP}, {5u, LINKUSB_GPIO_PULL_UP}, {6u, LINKUSB_GPIO_PULL_UP}},
    {{7u, LINKUSB_GPIO_PULL_UP}, {8u, LINKUSB_GPIO_PULL_UP}, {9u, LINKUSB_GPIO_PULL_UP}}
};

static bool valid(const linkusb_subsystem_t *s, uint32_t id, linkusb_subsystem_signal_t sig) {
    return s != 0 && s->initialized && id < LINKUSB_SUBSYSTEM_COUNT &&
           (uint32_t)sig < LINKUSB_SUBSYSTEM_SIGNAL_COUNT;
}
static const linkusb_subsystem_pin_config_t *config_for(uint32_t id, linkusb_subsystem_signal_t sig) {
    return &linkusb_subsystem_pin_map[id][(uint32_t)sig];
}
static void apply_pull(uint pin, linkusb_gpio_pull_t pull) {
    switch (pull) {
    case LINKUSB_GPIO_PULL_UP:
        gpio_set_pulls(pin, true, false);
        break;
    case LINKUSB_GPIO_PULL_DOWN:
        gpio_set_pulls(pin, false, true);
        break;
    case LINKUSB_GPIO_PULL_NONE:
    default:
        gpio_disable_pulls(pin);
        break;
    }
}
static void release_pin(const linkusb_subsystem_pin_config_t *config) {
    gpio_set_dir(config->gpio, GPIO_IN);
    apply_pull(config->gpio, config->pull);
}
bool linkusb_subsystem_init(linkusb_subsystem_t *s) {
    if (s == 0) return false;
    s->initialized = false;
    for (uint32_t id=0; id<LINKUSB_SUBSYSTEM_COUNT; ++id)
        for (uint32_t sig=0; sig<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++sig) {
            const linkusb_subsystem_pin_config_t *config = config_for(id, (linkusb_subsystem_signal_t)sig);
            gpio_init(config->gpio);
            gpio_put(config->gpio, 0u);
            release_pin(config);
        }
    s->initialized = true;
    return true;
}
void linkusb_subsystem_deinit(linkusb_subsystem_t *s) {
    if (s == 0) return;
    for (uint32_t id=0; id<LINKUSB_SUBSYSTEM_COUNT; ++id)
        for (uint32_t sig=0; sig<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++sig)
            release_pin(config_for(id, (linkusb_subsystem_signal_t)sig));
    s->initialized = false;
}
bool linkusb_subsystem_assert_signal(const linkusb_subsystem_t *s, uint32_t id, linkusb_subsystem_signal_t sig) {
    if (!valid(s,id,sig)) return false;
    const linkusb_subsystem_pin_config_t *config = config_for(id, sig);
    gpio_put(config->gpio, 0u);
    gpio_set_dir(config->gpio, GPIO_OUT);
    return true;
}
bool linkusb_subsystem_release_signal(const linkusb_subsystem_t *s, uint32_t id, linkusb_subsystem_signal_t sig) {
    if (!valid(s,id,sig)) return false;
    release_pin(config_for(id, sig));
    return true;
}
bool linkusb_subsystem_read(const linkusb_subsystem_t *s, uint32_t id, linkusb_subsystem_signal_t sig, bool *asserted) {
    if (!valid(s,id,sig) || asserted == 0) return false;
    *asserted = gpio_get(config_for(id, sig)->gpio) == 0u;
    return true;
}
uint8_t linkusb_subsystem_status(const linkusb_subsystem_t *s, uint32_t id) {
    uint8_t status=0u; bool asserted;
    if (s == 0 || !s->initialized || id >= LINKUSB_SUBSYSTEM_COUNT) return 0u;
    if (linkusb_subsystem_read(s,id,LINKUSB_SUBSYSTEM_RESET,&asserted) && asserted) status |= LINKUSB_SUBSYSTEM_STATUS_RESET;
    if (linkusb_subsystem_read(s,id,LINKUSB_SUBSYSTEM_ANALYSE,&asserted) && asserted) status |= LINKUSB_SUBSYSTEM_STATUS_ANALYSE;
    if (linkusb_subsystem_read(s,id,LINKUSB_SUBSYSTEM_ERROR,&asserted) && asserted) status |= LINKUSB_SUBSYSTEM_STATUS_ERROR;
    return status;
}
