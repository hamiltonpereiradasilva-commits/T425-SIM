#pragma once
#include <stdint.h>
#include <stdbool.h>
#define T425_OCCAM_MAX_PROCS 8
#define T425_OCCAM_MAX_OPS 128

typedef enum {
    OSEQ=0, OIN, OOUT, OALT, OTIMER_AFTER, OEND
} t425_occam_op_t;

typedef struct {
    t425_occam_op_t op;
    uint16_t arg0;
    uint16_t arg1;
    uint32_t imm;
} t425_occam_instr_t;

typedef struct {
    const char *name;
    const t425_occam_instr_t *code;
    uint16_t length;
} t425_occam_program_t;

bool t425_occam_validate(const t425_occam_program_t *p);
