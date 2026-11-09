#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"
#include "hardware/structs/padsbank0.h"
#include "linkusb_subsystem.h"

static linkusb_subsystem_t g_subsystem;
static const linkusb_subsystem_signal_t g_signals[LINKUSB_SUBSYSTEM_SIGNAL_COUNT] = { LINKUSB_SUBSYSTEM_RESET, LINKUSB_SUBSYSTEM_ANALYSE, LINKUSB_SUBSYSTEM_ERROR };
static const char *const g_names[LINKUSB_SUBSYSTEM_COUNT][LINKUSB_SUBSYSTEM_SIGNAL_COUNT] = {
    {"L0_RESET","L0_ANALYSE","L0_ERROR"}, {"L1_RESET","L1_ANALYSE","L1_ERROR"}
};
static uint gpio_for(uint32_t subsystem, linkusb_subsystem_signal_t signal) { return linkusb_subsystem_pin_map[subsystem][(uint32_t)signal].gpio; }

static bool pull_up_enabled(uint pin) { return (pads_bank0_hw->io[pin] & PADS_BANK0_GPIO0_PUE_BITS) != 0u; }
static bool pull_down_enabled(uint pin) { return (pads_bank0_hw->io[pin] & PADS_BANK0_GPIO0_PDE_BITS) != 0u; }

static bool inspect(uint32_t subsystem, linkusb_subsystem_signal_t signal, bool expected_asserted, bool expected_output) {
    uint pin = gpio_for(subsystem, signal);
    bool physical_high = gpio_get(pin) != 0u;
    bool output_enable = gpio_get_dir(pin) != 0u;
    bool pull_up = pull_up_enabled(pin);
    bool pull_down = pull_down_enabled(pin);
    bool latch_high = (sio_hw->gpio_out & (1u << pin)) != 0u;
    bool asserted = false;
    bool api_ok = linkusb_subsystem_read(&g_subsystem, subsystem, signal, &asserted);
    bool latch_low = !latch_high;
    bool api_required = api_ok && (asserted == expected_asserted);
    bool structural_ok = api_required && output_enable == expected_output && latch_low && pull_up && !pull_down;
    bool physical_ok = expected_asserted ? !physical_high : physical_high;
    bool ok = structural_ok && physical_ok;
    printf("%s oe=%u out=%u pu=%u pd=%u pin=%u read=%u structure=%s physical=%s %s\n", g_names[subsystem][signal], output_enable ? 1u : 0u, latch_high ? 1u : 0u, pull_up ? 1u : 0u, pull_down ? 1u : 0u, physical_high ? 1u : 0u, asserted ? 1u : 0u, structural_ok ? "PASS" : "FAIL", physical_ok ? "PASS" : "FAIL", ok ? "PASS" : "FAIL");
    return ok;
}
static bool inspect_deinit(uint32_t subsystem, linkusb_subsystem_signal_t signal) {
    uint pin = gpio_for(subsystem, signal);
    bool physical_high = gpio_get(pin) != 0u;
    bool output_enable = gpio_get_dir(pin) != 0u;
    bool pull_up = pull_up_enabled(pin);
    bool pull_down = pull_down_enabled(pin);
    bool latch_high = (sio_hw->gpio_out & (1u << pin)) != 0u;
    bool ok = !output_enable && !latch_high && pull_up && !pull_down && physical_high;
    printf("%s oe=%u out=%u pu=%u pd=%u pin=%u %s\n", g_names[subsystem][signal], output_enable ? 1u : 0u, latch_high ? 1u : 0u, pull_up ? 1u : 0u, pull_down ? 1u : 0u, physical_high ? 1u : 0u, ok ? "PASS" : "FAIL");
    return ok;
}
static bool phase_b(void) {
    bool ok = true; printf("[PHASE B] INDIVIDUAL SIGNALS\n");
    for (uint32_t subsystem=0; subsystem<LINKUSB_SUBSYSTEM_COUNT; ++subsystem)
        for (uint32_t i=0; i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++i) {
            linkusb_subsystem_signal_t sig=g_signals[i];
            ok = linkusb_subsystem_assert_signal(&g_subsystem,subsystem,sig) && ok;
            sleep_ms(5); ok = inspect(subsystem,sig,true,true) && ok;
            for (uint32_t os=0; os<LINKUSB_SUBSYSTEM_COUNT; ++os)
                for (uint32_t oi=0; oi<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++oi)
                    if (os!=subsystem || oi!=i) ok = inspect(os,g_signals[oi],false,false) && ok;
            ok = linkusb_subsystem_release_signal(&g_subsystem,subsystem,sig) && ok;
            sleep_ms(5); ok = inspect(subsystem,sig,false,false) && ok;
        }
    return ok;
}
static bool phase_c(void) {
    bool ok=true; printf("[PHASE C] SAME-SUBSYSTEM ISOLATION\n");
    ok &= linkusb_subsystem_assert_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_RESET);
    ok &= linkusb_subsystem_assert_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_ANALYSE);
    sleep_ms(5);
    ok &= inspect(0,LINKUSB_SUBSYSTEM_RESET,true,true) && inspect(0,LINKUSB_SUBSYSTEM_ANALYSE,true,true) && inspect(0,LINKUSB_SUBSYSTEM_ERROR,false,false);
    ok &= inspect(1,LINKUSB_SUBSYSTEM_RESET,false,false) && inspect(1,LINKUSB_SUBSYSTEM_ANALYSE,false,false) && inspect(1,LINKUSB_SUBSYSTEM_ERROR,false,false);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_RESET);
    ok &= inspect(0,LINKUSB_SUBSYSTEM_RESET,false,false) && inspect(0,LINKUSB_SUBSYSTEM_ANALYSE,true,true);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_ANALYSE);
    for (uint32_t i=0;i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT;++i) ok &= linkusb_subsystem_assert_signal(&g_subsystem,1,g_signals[i]);
    sleep_ms(5);
    for (uint32_t i=0;i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT;++i) ok &= inspect(1,g_signals[i],true,true);
    for (uint32_t i=0;i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT;++i) ok &= linkusb_subsystem_release_signal(&g_subsystem,1,g_signals[i]);
    return ok;
}
static bool phase_d(void) {
    bool ok=true; printf("[PHASE D] CROSS-SUBSYSTEM ISOLATION\n");
    ok &= linkusb_subsystem_assert_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_RESET);
    ok &= linkusb_subsystem_assert_signal(&g_subsystem,1,LINKUSB_SUBSYSTEM_ANALYSE);
    sleep_ms(5);
    ok &= inspect(0,LINKUSB_SUBSYSTEM_RESET,true,true) && inspect(1,LINKUSB_SUBSYSTEM_ANALYSE,true,true);
    ok &= inspect(0,LINKUSB_SUBSYSTEM_ANALYSE,false,false) && inspect(1,LINKUSB_SUBSYSTEM_RESET,false,false);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_RESET);
    ok &= inspect(0,LINKUSB_SUBSYSTEM_RESET,false,false) && inspect(1,LINKUSB_SUBSYSTEM_ANALYSE,true,true);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,1,LINKUSB_SUBSYSTEM_ANALYSE);
    return ok;
}
static bool status_line(uint32_t subsystem, linkusb_subsystem_signal_t signal,
                        uint8_t mask, bool expected_asserted) {
    bool asserted = false;
    bool api_ok = linkusb_subsystem_read(&g_subsystem, subsystem, signal, &asserted);
    uint8_t status = linkusb_subsystem_status(&g_subsystem, subsystem);
    bool status_asserted = (status & mask) != 0u;
    bool physical_high = gpio_get(gpio_for(subsystem, signal)) != 0u;
    bool physical_ok = expected_asserted ? !physical_high : physical_high;
    bool pass = api_ok && asserted == expected_asserted &&
                status_asserted == expected_asserted && physical_ok;
    printf("%s status=0x%02x read=%u physical=%u expected=%u %s\n",
           g_names[subsystem][signal], status, asserted ? 1u : 0u,
           physical_high ? 1u : 0u, expected_asserted ? 1u : 0u,
           pass ? "PASS" : "FAIL");
    return pass;
}
static bool phase_e(void) {
    bool ok=true; printf("[PHASE E] STATUS\n");
    for (uint32_t s=0; s<LINKUSB_SUBSYSTEM_COUNT; ++s) {
        ok &= status_line(s, LINKUSB_SUBSYSTEM_RESET, 0x01u, false);
        ok &= status_line(s, LINKUSB_SUBSYSTEM_ANALYSE, 0x02u, false);
        ok &= status_line(s, LINKUSB_SUBSYSTEM_ERROR, 0x04u, false);
    }
    ok &= linkusb_subsystem_assert_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_RESET);
    ok &= status_line(0, LINKUSB_SUBSYSTEM_RESET, 0x01u, true);
    ok &= status_line(1, LINKUSB_SUBSYSTEM_RESET, 0x01u, false);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_RESET);
    ok &= status_line(0, LINKUSB_SUBSYSTEM_RESET, 0x01u, false);

    ok &= linkusb_subsystem_assert_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_ANALYSE);
    ok &= status_line(0, LINKUSB_SUBSYSTEM_ANALYSE, 0x02u, true);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_ANALYSE);
    ok &= status_line(0, LINKUSB_SUBSYSTEM_ANALYSE, 0x02u, false);

    ok &= linkusb_subsystem_assert_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_ERROR);
    ok &= status_line(0, LINKUSB_SUBSYSTEM_ERROR, 0x04u, true);
    ok &= linkusb_subsystem_release_signal(&g_subsystem,0,LINKUSB_SUBSYSTEM_ERROR);
    ok &= status_line(0, LINKUSB_SUBSYSTEM_ERROR, 0x04u, false);

    for (uint32_t s=0; s<LINKUSB_SUBSYSTEM_COUNT; ++s) {
        for (uint32_t i=0; i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++i)
            ok &= linkusb_subsystem_assert_signal(&g_subsystem,s,g_signals[i]);
        for (uint32_t i=0; i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++i)
            ok &= status_line(s,g_signals[i],(uint8_t)(1u<<i),true);
        for (uint32_t i=0; i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++i)
            ok &= linkusb_subsystem_release_signal(&g_subsystem,s,g_signals[i]);
        for (uint32_t i=0; i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT; ++i)
            ok &= status_line(s,g_signals[i],(uint8_t)(1u<<i),false);
    }
    return ok;
}
int main(void) {
    stdio_init_all(); while (!stdio_usb_connected()) sleep_ms(10); sleep_ms(250);
    bool ok=true;
    printf("\n===== USB-LINK-04E SUBSYSTEM HW =====\nPINMAP:\nL0_RESET=GPIO4\nL0_ANALYSE=GPIO5\nL0_ERROR=GPIO6\nL1_RESET=GPIO7\nL1_ANALYSE=GPIO8\nL1_ERROR=GPIO9\nEXTERNAL_PULLUPS_REQUIRED=NO\nINTERNAL_PULLUPS_USED=YES\n");
    printf("[PHASE A] INITIAL RELEASE\n"); ok &= linkusb_subsystem_init(&g_subsystem);
    for (uint32_t s=0;s<LINKUSB_SUBSYSTEM_COUNT;++s) for (uint32_t i=0;i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT;++i) ok &= inspect(s,g_signals[i],false,false);
    ok &= phase_b() && phase_c() && phase_d() && phase_e();
    printf("[PHASE F] DEINIT\n"); linkusb_subsystem_deinit(&g_subsystem);
    for (uint32_t s=0;s<LINKUSB_SUBSYSTEM_COUNT;++s) for (uint32_t i=0;i<LINKUSB_SUBSYSTEM_SIGNAL_COUNT;++i) ok &= inspect_deinit(s,g_signals[i]);
    printf("BOOT_STEADY_STATE_RELEASED=YES\nBOOT_TRANSIENT_GLITCH_PROVEN=NO_REQUIRES_EXTERNAL_INSTRUMENTATION\n");
    printf(ok ? "===== USB-LINK-04E PASS =====\n" : "===== USB-LINK-04E FAIL =====\n");
    while (true) tight_loop_contents();
}
