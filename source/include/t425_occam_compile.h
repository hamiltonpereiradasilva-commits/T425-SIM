#pragma once
#include <stdbool.h>
#include "t425_occam_ast.h"
#include "t425_occam.h"
bool t425_occam_compile(const t425_ast_t *ast,t425_occam_instr_t *out,
                        uint16_t cap,uint16_t *n);
