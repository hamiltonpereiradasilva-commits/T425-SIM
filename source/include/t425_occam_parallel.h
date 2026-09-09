#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "t425_occam_ast.h"
#include "t425_occam.h"

#define T425_MAX_PROCESSES 16
#define T425_MAX_IR_PER_PROCESS 128

typedef struct {
    uint16_t pid;
    uint16_t ir_offset;
    uint16_t ir_count;
} t425_process_image_t;

typedef struct {
    t425_process_image_t process[T425_MAX_PROCESSES];
    t425_occam_instr_t ir[T425_MAX_PROCESSES*T425_MAX_IR_PER_PROCESS];
    uint16_t process_count;
    uint16_t ir_count;
} t425_parallel_image_t;

bool t425_occam_compile_parallel(const t425_ast_t *ast,
                                 t425_parallel_image_t *image);
