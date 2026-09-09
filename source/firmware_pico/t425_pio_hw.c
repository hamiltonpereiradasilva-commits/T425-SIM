#include "t425_pio_hw.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "t425_uart8_8n1.pio.h"

static PIO pios[T425_HW_LINKS];
static uint sm_tx[T425_HW_LINKS], sm_rx[T425_HW_LINKS];
static bool ready[T425_HW_LINKS];

static void setup_tx(PIO pio, uint sm, uint off, uint pin, uint baud) {
    pio_sm_config c=t425_uart_tx_program_get_default_config(off);
    sm_config_set_out_pins(&c,pin,1);
    sm_config_set_out_shift(&c,true,false,8);
    sm_config_set_fifo_join(&c,PIO_FIFO_JOIN_TX);
    sm_config_set_clkdiv(&c,(float)clock_get_hz(clk_sys)/((float)baud*8.0f));
    pio_gpio_init(pio,pin);
    pio_sm_set_consecutive_pindirs(pio,sm,pin,1,true);
    pio_sm_init(pio, sm, off, &c);
    pio_sm_set_pins_with_mask(pio,sm,1u<<pin,1u<<pin);
    pio_sm_set_enabled(pio,sm,true);
}
static void setup_rx(PIO pio, uint sm, uint off, uint pin, uint baud) {
    pio_sm_config c=t425_uart_rx_program_get_default_config(off);
    sm_config_set_in_pins(&c,pin);
    sm_config_set_in_shift(&c,true,true,8);
    sm_config_set_fifo_join(&c,PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv(&c,(float)clock_get_hz(clk_sys)/((float)baud*8.0f));
    pio_gpio_init(pio,pin);
    pio_sm_set_consecutive_pindirs(pio,sm,pin,1,false);
    gpio_pull_up(pin);
    pio_sm_init(pio, sm, off, &c);
    pio_sm_set_enabled(pio,sm,true);
}
bool t425_pio_hw_init(const t425_pio_hw_cfg_t cfg[T425_HW_LINKS]) {
    for(uint i=0;i<T425_HW_LINKS;i++){
        if(cfg[i].tx_pin==cfg[i].rx_pin) return false;
        PIO p=cfg[i].pio_index?pio1:pio0;
        int tx=pio_claim_unused_sm(p,false), rx=pio_claim_unused_sm(p,false);
        if(tx<0||rx<0) return false;
        uint ot=pio_add_program(p,&t425_uart_tx_program);
        uint orx=pio_add_program(p,&t425_uart_rx_program);
        pios[i]=p; sm_tx[i]=(uint)tx; sm_rx[i]=(uint)rx;
        setup_tx(p,sm_tx[i],ot,cfg[i].tx_pin,cfg[i].baud);
        setup_rx(p,sm_rx[i],orx,cfg[i].rx_pin,cfg[i].baud);
        ready[i]=true;
    }
    return true;
}
bool t425_pio_hw_write(uint8_t link,const uint8_t*d,uint16_t n){
    if(link>=T425_HW_LINKS||!ready[link]||!d)return false;
    for(uint16_t i=0;i<n;i++) pio_sm_put_blocking(pios[link],sm_tx[link],d[i]);
    return true;
}
uint16_t t425_pio_hw_read(uint8_t link,uint8_t*d,uint16_t max){
    if(link>=T425_HW_LINKS||!ready[link]||!d)return 0;
    uint16_t n=0;
    while(n<max&&!pio_sm_is_rx_fifo_empty(pios[link],sm_rx[link]))
        d[n++]=(uint8_t)pio_sm_get(pios[link],sm_rx[link]);
    return n;
}
bool t425_pio_hw_ready(uint8_t link){return link<T425_HW_LINKS&&ready[link];}
