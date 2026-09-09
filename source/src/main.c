#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "t425_config.h"
#include "t425_vm.h"
#include "t425_scheduler.h"
#include "t425_links.h"
#include "t425_protocol.h"
#include "t425_selftest.h"

static t425_vm_t vm;

int main(void){
    stdio_init_all(); sleep_ms(1000);
    gpio_init(T425_LED_PIN);gpio_set_dir(T425_LED_PIN,GPIO_OUT);
    t425_vm_reset(&vm); t425_scheduler_init(&vm); t425_links_init(); t425_protocol_init();

    bool ok=t425_selftest();
    printf("\nT425-SIM RP2040 NODE P%02d\n",T425_NODE_ID);
    printf("ISA smoke test: %s\n",ok?"PASS":"FAIL");
    printf("T425 RAM 4 KiB: 0x%08X..0x%08X\n",T425_INT_RAM_BASE,T425_INT_RAM_END);
    printf("MemStart: 0x%08X\n",T425_MEMSTART);
    printf("Links: 4 x laboratory transport; INMOS-compatible implementation is next hardware milestone\n");
    printf("Management UART: %u baud TX=%u RX=%u DE=%u\n",
           T425_MGMT_BAUD,T425_MGMT_TX_PIN,T425_MGMT_RX_PIN,T425_MGMT_DE_PIN);

    while(true){
        t425_protocol_poll(&vm);
        if(!vm.cpu.halted){t425_scheduler_tick(&vm);t425_vm_run_slice(&vm,64);}
        t425_vm_tick(&vm);
        gpio_put(T425_LED_PIN,(get_absolute_time()._private_us>>20)&1);
        tight_loop_contents();
    }
}
