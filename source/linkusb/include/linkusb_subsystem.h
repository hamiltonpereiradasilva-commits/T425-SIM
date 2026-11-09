#ifndef LINKUSB_SUBSYSTEM_H
#define LINKUSB_SUBSYSTEM_H
#include <stdbool.h>
#include <stdint.h>
#define LINKUSB_SUBSYSTEM_COUNT 2u
#define LINKUSB_SUBSYSTEM_SIGNAL_COUNT 3u
#define LINKUSB_SUBSYSTEM_STATUS_RESET 0x01u
#define LINKUSB_SUBSYSTEM_STATUS_ANALYSE 0x02u
#define LINKUSB_SUBSYSTEM_STATUS_ERROR 0x04u
#define LINKUSB_SUBSYSTEM_STATUS_FAULT 0x08u
typedef enum {
    LINKUSB_GPIO_PULL_NONE = 0,
    LINKUSB_GPIO_PULL_UP = 1,
    LINKUSB_GPIO_PULL_DOWN = 2
} linkusb_gpio_pull_t;
typedef enum {
    LINKUSB_SUBSYSTEM_RESET = 0,
    LINKUSB_SUBSYSTEM_ANALYSE = 1,
    LINKUSB_SUBSYSTEM_ERROR = 2
} linkusb_subsystem_signal_t;
typedef struct {
    uint8_t gpio;
    linkusb_gpio_pull_t pull;
} linkusb_subsystem_pin_config_t;
typedef struct { bool initialized; } linkusb_subsystem_t;
extern const linkusb_subsystem_pin_config_t linkusb_subsystem_pin_map[LINKUSB_SUBSYSTEM_COUNT][LINKUSB_SUBSYSTEM_SIGNAL_COUNT];
bool linkusb_subsystem_init(linkusb_subsystem_t *subsystem);
void linkusb_subsystem_deinit(linkusb_subsystem_t *subsystem);
bool linkusb_subsystem_assert_signal(const linkusb_subsystem_t *subsystem, uint32_t subsystem_id, linkusb_subsystem_signal_t signal);
bool linkusb_subsystem_release_signal(const linkusb_subsystem_t *subsystem, uint32_t subsystem_id, linkusb_subsystem_signal_t signal);
bool linkusb_subsystem_read(const linkusb_subsystem_t *subsystem, uint32_t subsystem_id, linkusb_subsystem_signal_t signal, bool *asserted);
uint8_t linkusb_subsystem_status(const linkusb_subsystem_t *subsystem, uint32_t subsystem_id);
#endif
