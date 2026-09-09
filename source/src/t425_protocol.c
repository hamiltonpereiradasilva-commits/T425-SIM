#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "t425_protocol.h"
#include "t425_debug.h"

#define MAX_PAYLOAD 128

static uint8_t rxbuf[MAX_PAYLOAD];

static void send_frame(uint8_t cmd, const uint8_t *p, uint8_t n) {
    gpio_put(T425_MGMT_DE_PIN, 1);
    uint8_t crc=(uint8_t)(cmd+n);
    for (uint8_t i=0;i<n;i++) crc=(uint8_t)(crc+p[i]);
    uart_putc_raw(T425_MGMT_UART, T425_MGMT_MAGIC0);
    uart_putc_raw(T425_MGMT_UART, T425_MGMT_MAGIC1);
    uart_putc_raw(T425_MGMT_UART, T425_MGMT_VERSION);
    uart_putc_raw(T425_MGMT_UART, T425_NODE_ID);
    uart_putc_raw(T425_MGMT_UART, cmd);
    uart_putc_raw(T425_MGMT_UART, n);
    for (uint8_t i=0;i<n;i++) uart_putc_raw(T425_MGMT_UART,p[i]);
    uart_putc_raw(T425_MGMT_UART,crc);
    uart_tx_wait_blocking(T425_MGMT_UART);
    gpio_put(T425_MGMT_DE_PIN, 0);
}

void t425_protocol_init(void) {
    uart_init(T425_MGMT_UART,T425_MGMT_BAUD);
    gpio_set_function(T425_MGMT_TX_PIN,GPIO_FUNC_UART);
    gpio_set_function(T425_MGMT_RX_PIN,GPIO_FUNC_UART);
    gpio_init(T425_MGMT_DE_PIN);
    gpio_set_dir(T425_MGMT_DE_PIN,GPIO_OUT);
    gpio_put(T425_MGMT_DE_PIN,0); /* receive by default */
}

static bool read_byte_timeout(uint8_t *b) {
    if (!uart_is_readable(T425_MGMT_UART)) return false;
    *b=uart_getc(T425_MGMT_UART); return true;
}

void t425_protocol_poll(t425_vm_t *vm) {
    static uint8_t state=0, ver=0, node=0, cmd=0, len=0, idx=0;
    static uint8_t crc=0;
    uint8_t b;
    while (read_byte_timeout(&b)) {
        switch(state) {
        case 0: state=(b==T425_MGMT_MAGIC0)?1:0; break;
        case 1: state=(b==T425_MGMT_MAGIC1)?2:0; break;
        case 2: ver=b; state=(ver==T425_MGMT_VERSION)?3:0; break;
        case 3: node=b; state=4; break;
        case 4: cmd=b; state=5; break;
        case 5:
            len=b; idx=0; state=(len<=MAX_PAYLOAD)?(len?6:7):0; break;
        case 6:
            rxbuf[idx++]=b; if(idx>=len) state=7; break;
        case 7:
            crc=b; state=0;
            switch(cmd) {
            case CMD_PING:
                send_frame(CMD_PING,(uint8_t[]){T425_NODE_ID},1); break;
            case CMD_RESET:
                t425_vm_reset(vm); send_frame(CMD_RESET,NULL,0); break;
            case CMD_RUN:
                t425_vm_run(vm); send_frame(CMD_RUN,NULL,0); break;
            case CMD_HALT:
                t425_vm_halt(vm); send_frame(CMD_HALT,NULL,0); break;
            case CMD_STEP:
                t425_vm_step(vm); send_frame(CMD_STEP,NULL,0); break;
            case CMD_READ_REGS: {
                uint32_t r[6]; t425_vm_get_regs(vm,r);
                send_frame(CMD_READ_REGS,(uint8_t*)r,24); break;
            }
            case CMD_READ_MEM:
                if(len>=8) {
                    uint32_t a=*(uint32_t*)&rxbuf[0], n=*(uint32_t*)&rxbuf[4];
                    if(n>MAX_PAYLOAD) n=MAX_PAYLOAD;
                    uint8_t tmp[MAX_PAYLOAD];
                    if(t425_vm_read_mem(vm,a,tmp,n)) send_frame(CMD_READ_MEM,tmp,(uint8_t)n);
                }
                break;
            case CMD_WRITE_MEM:
                if(len>=4) {
                    uint32_t a=*(uint32_t*)&rxbuf[0];
                    t425_vm_write_mem(vm,a,&rxbuf[4],len-4);
                    send_frame(CMD_WRITE_MEM,NULL,0);
                }
                break;
            case CMD_LOAD:
                if(len>=4) {
                    uint32_t a=*(uint32_t*)&rxbuf[0];
                    t425_vm_load(vm,a,&rxbuf[4],len-4);
                    send_frame(CMD_LOAD,NULL,0);
                }
                break;
            case CMD_STATUS: {
                uint8_t s[17]={0};
                uint32_t r[6]; t425_vm_get_regs(vm,r);
                memcpy(s,r,24>16?16:16);
                s[16]=(uint8_t)(vm->cpu.error?1:0);
                send_frame(CMD_STATUS,s,sizeof(s));
                break;
            }
            case CMD_ANALYSE:
                vm->analyse=(len>0 && rxbuf[0])?1:0;
                if(vm->analyse) t425_vm_halt(vm);
                send_frame(CMD_ANALYSE,NULL,0);
                break;
            default: break;
            }
            break;
        default: state=0; break;
        }
    }
}
