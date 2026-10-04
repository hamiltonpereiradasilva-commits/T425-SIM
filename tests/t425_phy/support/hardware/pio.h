#ifndef T425_TEST_PIO_H
#define T425_TEST_PIO_H
#include <stdbool.h>
#include <stdint.h>
typedef unsigned int uint;
typedef struct test_pio_hw {
    uint32_t txf[4];
    uint32_t rxf[4];
} test_pio_hw_t;
typedef test_pio_hw_t *PIO;
typedef struct pio_program {
    const uint16_t *instructions;
    uint length;
    int origin;
    uint pio_version;
} pio_program;
typedef struct {
    uint wrap_target;
    uint wrap;
} pio_sm_config;
extern PIO test_pio;
static inline pio_sm_config pio_get_default_sm_config(void) { pio_sm_config c = {0u, 0u}; return c; }
static inline void sm_config_set_wrap(pio_sm_config *c, uint a, uint b) { c->wrap_target = a; c->wrap = b; }
static inline void sm_config_set_out_pins(pio_sm_config *c, uint p, uint n) { (void)c; (void)p; (void)n; }
static inline void sm_config_set_out_shift(pio_sm_config *c, bool r, bool a, uint n) { (void)c; (void)r; (void)a; (void)n; }
static inline void sm_config_set_in_pins(pio_sm_config *c, uint p) { (void)c; (void)p; }
static inline void sm_config_set_jmp_pin(pio_sm_config *c, uint p) { (void)c; (void)p; }
static inline void sm_config_set_in_shift(pio_sm_config *c, bool r, bool a, uint n) { (void)c; (void)r; (void)a; (void)n; }
static inline void sm_config_set_clkdiv(pio_sm_config *c, float d) { (void)c; (void)d; }
uint pio_add_program(PIO pio, const pio_program *program);
void pio_gpio_init(PIO pio, uint pin);
void pio_sm_set_consecutive_pindirs(PIO pio, uint sm, uint pin, uint count, bool out);
void pio_sm_init(PIO pio, uint sm, uint offset, const pio_sm_config *config);
void pio_enable_sm_mask_in_sync(PIO pio, uint mask);
void pio_set_sm_mask_enabled(PIO pio, uint mask, bool enabled);
uint pio_get_dreq(PIO pio, uint sm, bool is_tx);
bool pio_sm_is_rx_fifo_empty(PIO pio, uint sm);
uint32_t pio_sm_get(PIO pio, uint sm);
#endif
