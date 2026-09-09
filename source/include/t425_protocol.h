#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_vm.h"

typedef enum {
    CMD_PING=0x01, CMD_RESET=0x02, CMD_RUN=0x03, CMD_HALT=0x04,
    CMD_STEP=0x05, CMD_READ_REGS=0x06, CMD_READ_MEM=0x07,
    CMD_WRITE_MEM=0x08, CMD_LOAD=0x09, CMD_STATUS=0x0A, CMD_ANALYSE=0x0B
} t425_cmd_t;

void t425_protocol_init(void);
void t425_protocol_poll(t425_vm_t *vm);
